#include <sys/statvfs.h>
#include <limits.h>
#include <napi.h>
#include "errno-error.h"

using namespace Napi;
using namespace Errno;

namespace StatVFS {

  class StatVFSAsyncWorker : public AsyncWorker {

    public:
      StatVFSAsyncWorker(String& path, Function& callback)
      : AsyncWorker(callback) {
        std::string str = path.Utf8Value();

        std::copy(str.begin(), str.end(), this->path);
        this->path[str.size()] = '\0';

        std::memset((void *) &stat, 0, sizeof(struct statvfs));
      };

      void Execute() {
        if (statvfs(path, &stat)) error = errno;
      };

      void OnOK() {
        class Env env = Env();

        if (error) {
          Callback().Call({ ErrnoError(env, error, "statvfs", path ).Value() });
        } else {
          Object result = Object::New(env);

          result.Set("f_bsize", (double) stat.f_bsize);     /* File system block size */
          result.Set("f_frsize", (double) stat.f_frsize);   /* Fundamental file system block size */
          result.Set("f_blocks", (double) stat.f_blocks);   /* Blocks on FS in units of f_frsize */
          result.Set("f_bfree", (double) stat.f_bfree);     /* Free blocks */
          result.Set("f_bavail", (double) stat.f_bavail);   /* Blocks available to non-root */
          result.Set("f_files", (double) stat.f_files);     /* Total inodes */
          result.Set("f_ffree", (double) stat.f_ffree);     /* Free inodes */
          result.Set("f_favail", (double) stat.f_favail);   /* Free inodes for non-root */
          result.Set("f_fsid", (double) stat.f_fsid);       /* Filesystem ID */
          result.Set("f_flag", (double) stat.f_flag);       /* Bit mask of values */
          result.Set("f_namemax", (double) stat.f_namemax); /* Max file name length */

          Callback().Call({ Env().Undefined(), result });
        }
      };

    private:
      char path[PATH_MAX];
      struct statvfs stat;
      int error = 0;
};

  Value startStatVFS(const CallbackInfo& info) {
    Env env = info.Env();

    if (info.Length() != 2) throw TypeError::New(env, "Two arguments required: path, callback");
    if (! info[0].IsString()) throw TypeError::New(env, "Path must be a string");
    if (! info[1].IsFunction()) throw TypeError::New(env, "Callback must be a function");

    String path = info[0].As<String>();
    Function callback = info[1].As<Function>();

    StatVFSAsyncWorker* worker = new StatVFSAsyncWorker(path, callback);
    worker->Queue();

    return env.Undefined();
  };

  Object Init(Env env, Object exports) {
    exports["statvfs"] = Function::New(env, startStatVFS, std::string("statvfs"));
    return exports;
  };

  NODE_API_MODULE(addon, Init)
} // namespace statvfs
