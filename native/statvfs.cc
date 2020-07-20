#include <node.h>
#include <uv.h>

#include <sys/statvfs.h>
#include <errno.h>


namespace node_statvfs {
    using node::ErrnoException;

    using v8::Exception;
    using v8::Function;
    using v8::FunctionCallbackInfo;
    using v8::HandleScope;
    using v8::Isolate;
    using v8::Local;
    using v8::NewStringType;
    using v8::Null;
    using v8::Number;
    using v8::Object;
    using v8::Persistent;
    using v8::String;
    using v8::Value;

    /* Define our structure for asynchronous processing */
    struct AsyncData {
        Persistent<Function> callback; // the callback invoked after statvfs
        const char * path; // the path to check for free space
        int64_t total; // total number of bytes in the filesystem
        int64_t avail; // available number of bytes in the filesystem
        int64_t free; // free (including restricted) bytes in the filesystem
        int error; // when non-zero this is errno from statvfs
    };

    static void StartAsync(const FunctionCallbackInfo<Value>& args);
    static void RunAsync(uv_work_t * request);
    static void FinishAsync(uv_work_t *request, int status);

    /* Start asynchronous processing: check arguments and schedule job */
    static void StartAsync(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();

        // Two arguments: path and callback
        if (args.Length() < 2) {
            isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate,
                                    "Two arguments (path and callback) required",
                                    NewStringType::kNormal).ToLocalChecked()));
            return;
        }

        if (!args[0]->IsString()) {
            isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate,
                                    "Wrong type for first argument (path)",
                                    NewStringType::kNormal).ToLocalChecked()));
            return;
        }

        if (!args[1]->IsFunction()) {
            isolate->ThrowException(Exception::TypeError(
                String::NewFromUtf8(isolate,
                                    "Wrong type for second argument (callback)",
                                    NewStringType::kNormal).ToLocalChecked()));
            return;
        }

        // Setup our "arguments" structure
        AsyncData * data = new AsyncData();

        String::Utf8Value path(isolate, args[0]);
        char *path_copy = (char *) calloc(path.length() + 1, sizeof(char));
        strcpy(path_copy, *path);
        data->path = path_copy;

        Local<Function> callback = Local<Function>::Cast(args[1]);
        data->callback.Reset(isolate, callback);

        data->error = 0;
        data->total = -1;
        data->avail = -1;
        data->free = -1;

        // Queue up our work request for asynchronous processing
        uv_work_t * request = new uv_work_t();
        request->data = data;
        uv_queue_work(uv_default_loop(), request, RunAsync, FinishAsync);

        // Always return "undefined"
        args.GetReturnValue().Set(Undefined(isolate));
    }

    /* Asynchronously run statvfs and set total and available bytes */
    static void RunAsync(uv_work_t * request) {
        AsyncData * data = (AsyncData *) request->data;
        struct statvfs buf;

        int error = statvfs(data->path, &buf);

        if (error != 0) {
            data->error = errno;
            data->total = -1;
            data->avail = -1;
            data->free = -1;
        } else {
            data->error = 0;
            data->total = buf.f_blocks * buf.f_frsize;
            data->avail = buf.f_bavail * buf.f_frsize;
            data->free = buf.f_bfree * buf.f_frsize;
        }
    }

    /* Finish up our asynchronous job invoking the callback */
    static void FinishAsync(uv_work_t *request, int status) {
        AsyncData * data = (AsyncData *) request->data;
        Isolate *isolate = Isolate::GetCurrent();
        HandleScope handleScope(isolate);

        const unsigned int argc = data->error ? 1 : 4;
        Local<Value> argv[argc];

        if (data->error) {
            argv[0] = ErrnoException(isolate, data->error, "statvfs", NULL, data->path);
        } else {
            argv[0] = Undefined(isolate);
            argv[1] = Number::New(isolate, data->total);
            argv[2] = Number::New(isolate, data->avail);
            argv[3] = Number::New(isolate, data->free);
        }

        (void) Local<Function>::New(isolate, data->callback)->Call(
            isolate->GetCurrentContext(),
            Null(isolate),
            argc,
            argv
        );

        // Cleanup
        free((void *) data->path);
        data->callback.Reset();
        delete data;
        delete request;
    }

    void Init(Local<Object> exports, Local<Object> module) {
        NODE_SET_METHOD(module, "exports", StartAsync);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
} // namespace statvfs
