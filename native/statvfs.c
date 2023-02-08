// standard lib imports
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// node/libuv imports
#include <node_api.h>
#include <uv.h>

// should be defined by gyp
#ifndef ADDON_VERSION
#define ADDON_VERSION "0.0.0"
#endif

/* ========================================================================== *
 * NAPI_CALL_VALUE / NAPI_CALL_VOID: wrapper to throw exception when a        *
 * `napi_...` call returns something different from `napi_ok`                 *
 * ========================================================================== */

/** Throw an error, used by NAPI_CALL_VALUE / NAPI_CALL_VOID */
static void _napi_call_error(
  napi_env _env,
  napi_status _status,
  const char * _call,
  int _line
) {
  if (_status == napi_ok) return;

  // Get the extended error info from the latest `napi_...` call
  const napi_extended_error_info* __error_info = NULL;
  _status = napi_get_last_error_info(_env, &__error_info);

  // Get the error message from the `napi_extended_error_info`
  const char *__error_message = NULL;
  if ((_status == napi_ok) && (__error_info->error_message != NULL)) {
    __error_message = __error_info->error_message;
  } else {
    __error_message = "Unknown Error";
  }

  // If no exception is pending already, create an error and throw it
  bool __is_pending = 0;
  _status = napi_is_exception_pending(_env, &__is_pending);
  if ((_status == napi_ok) && (__is_pending == 0)) {
    char __buffer[128];
    snprintf(__buffer, sizeof(__buffer), "%s (%s, line=%d)", __error_message, _call, _line);
    napi_throw_error(_env, NULL, __buffer);
  }
}

/** Wrap a call to a `napi_...` function and throw + return NULL on failure */
#define NAPI_CALL_VALUE(_call, _env, ...) \
  do { \
    napi_status __status = _call(_env, __VA_ARGS__); \
    if (__status == napi_ok) break; \
    _napi_call_error(_env, __status, #_call, __LINE__); \
    return NULL; \
  } while (0);

/** Wrap a call to a `napi_...` function and throw + return on failure */
#define NAPI_CALL_VOID(_call, _env, ...) \
  do { \
    napi_status __status = _call(_env, __VA_ARGS__); \
    if (__status == napi_ok) break; \
    _napi_call_error(_env, __status, #_call, __LINE__); \
    return; \
  } while (0);

/* ========================================================================== */

/** Create a _system_ error, from a standard (or `libuv`) errno */
static napi_value _system_error(
  napi_env _env,
  const char * _syscall,
  int _errno
) {
  const char * __message_chars = NULL;
  const char * __code_chars = NULL;

  // Our `errno` is very likely a system errno, let's translate it to a libuv
  // error and get the message / code, otherwise it's "unknown"...
  if (_errno != 0) {
    const int __errno = uv_translate_sys_error(_errno);
    __message_chars = uv_strerror(__errno);
    __code_chars = uv_err_name(__errno);
  } else {
    __message_chars = "Unknown Error";
  }

  // Convert message and code into JS strings
  napi_value __message = NULL;
  if (__message_chars != NULL) {
    NAPI_CALL_VALUE(napi_create_string_latin1, _env, __message_chars, NAPI_AUTO_LENGTH, &__message);
  }

  napi_value __code = NULL;
  if (__code_chars != NULL) {
    NAPI_CALL_VALUE(napi_create_string_latin1, _env, __code_chars, NAPI_AUTO_LENGTH, &__code);
  }

  // Create our error object
  napi_value __error = NULL;
  NAPI_CALL_VALUE(napi_create_error, _env, __code, __message, &__error);

  // Inject the "errno" property in our error
  napi_value __errno = NULL;
  NAPI_CALL_VALUE(napi_create_uint32, _env, _errno, &__errno);
  NAPI_CALL_VALUE(napi_set_named_property, _env, __error, "errno", __errno);

  // If we know the syscall, inject that as a string into our error
  if (_syscall != NULL) {
    napi_value __syscall = NULL;
    NAPI_CALL_VALUE(napi_create_string_latin1, _env, _syscall, NAPI_AUTO_LENGTH, &__syscall);
    NAPI_CALL_VALUE(napi_set_named_property, _env, __error, "syscall", __syscall);
  }

  // Return our error
  return __error;
}

/* ========================================================================== */

/** Throw a type error with a specified message */
static void _throw_type_error(
  napi_env _env,
  const char *_message
) {
  bool __is_pending = false;
  napi_status __status = napi_is_exception_pending(_env, &__is_pending);
  if ((__status == napi_ok) && (__is_pending == false)) {
    const char *__message = (_message == NULL) ? "" : _message;
    napi_throw_type_error(_env, NULL, __message);
  }
}

/* ========================================================================== *
 * OPEN: asynchronously open our socket                                       *
 * ========================================================================== */

/** Data to pass around in `open` asynchronous work */
struct _statvfs_data {
  /** The `napi_async_work` structure associated with this `open` operation */
  napi_async_work __async_work;
  /** A reference to the JavaScript callback function to invoke after `open` */
  napi_ref __callback_ref;
  /** The mounted path where stats will be collected from */
  char __path[PATH_MAX];
  /** Structure receiving our stats */
  struct statvfs __stats;
  /** A boolean indicating success or not*/
  bool __success;
  /** Either `0` or the `errno` from the sytem call that failed */
  int __errno;
};

/* ========================================================================== */

/**
 * Invoke `statvfs` and record our stats
 */
static void _statvfs_execute(
  napi_env _env,
  void *_data
) {
  struct _statvfs_data * __data = (struct _statvfs_data *) _data;

  if (statvfs(__data->__path, &__data->__stats)) {
    __data->__success = false;
    __data->__errno = errno;
  } else {
    __data->__success = true;
    __data->__errno = errno;
  }
}

/* ========================================================================== */

/** Complete our call and invoke the JavaScript callback */
static void _statvfs_complete(
  napi_env _env,
  napi_status _status,
  void* data
) {
  // Copy the data allocated in `_statvfs` and free its pointer
  struct _statvfs_data __data;
  memcpy(&__data, data, sizeof(struct _statvfs_data));
  free(data);

  // Get JS's `null` and `undefined`, then prep the arguments for the callback
  napi_value __args[2];
  NAPI_CALL_VOID(napi_get_null, _env, &__args[0]);
  NAPI_CALL_VOID(napi_get_undefined, _env, &__args[1]);

  // If we have a negative file descriptor, we have an error and pass it to our
  // callback as the _first_ argument, otherwise we pass the file descriptor as
  // the second argument to the callback.
  if (_status != napi_ok) {
    char __message_chars[128];
    snprintf(__message_chars, sizeof(__message_chars), "NAPI error (status=%d)", _status);

    napi_value __message = NULL;
    NAPI_CALL_VOID(napi_create_string_latin1, _env, __message_chars, NAPI_AUTO_LENGTH, &__message);

    NAPI_CALL_VOID(napi_create_error, _env, NULL, __message, &__args[0]);
  } else if (__data.__success) {
    napi_value __f_bsize = NULL;
    napi_value __f_blocks = NULL;
    napi_value __f_bfree = NULL;
    napi_value __f_bavail = NULL;
    napi_value __f_files = NULL;
    napi_value __f_ffree = NULL;

    NAPI_CALL_VOID(napi_create_object, _env, &__args[1]);

    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_bsize, &__f_bsize);
    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_blocks, &__f_blocks);
    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_bfree, &__f_bfree);
    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_bavail, &__f_bavail);
    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_files, &__f_files);
    NAPI_CALL_VOID(napi_create_bigint_uint64, _env, __data.__stats.f_ffree, &__f_ffree);

    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_bsize", __f_bsize);
    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_blocks", __f_blocks);
    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_bfree", __f_bfree);
    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_bavail", __f_bavail);
    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_files", __f_files);
    NAPI_CALL_VOID(napi_set_named_property, _env, __args[1], "f_ffree", __f_ffree);
  } else {
    __args[0] = _system_error(_env, "statvfs", __data.__errno);
  }

  // Get our callback function
  napi_value __callback = NULL;
  NAPI_CALL_VOID(napi_get_reference_value, _env, __data.__callback_ref, &__callback);

  // Call our callback with our arguments, scoped in `global`
  napi_value __global = NULL;
  NAPI_CALL_VOID(napi_get_global, _env, &__global);
  NAPI_CALL_VOID(napi_call_function, _env, __global, __callback, 2, __args, NULL);

  // Cleanup: delete reference to our callback and our async work
  NAPI_CALL_VOID(napi_delete_reference, _env, __data.__callback_ref);
  NAPI_CALL_VOID(napi_delete_async_work, _env, __data.__async_work);
}

/* ========================================================================== */

/** Initiate our asynchronous `open` call */
static napi_value _statvfs(
  napi_env _env,
  napi_callback_info _info
) {
  napi_valuetype __type = napi_undefined;

  // Allocate _statvfs_data here, it'll be malloc'ed later
  struct _statvfs_data __data;
  bzero(&__data, sizeof(struct _statvfs_data));

  // Get our `open` call arguments
  size_t __argc = 2;
  napi_value __args[2];
  NAPI_CALL_VALUE(napi_get_cb_info, _env, _info, &__argc, __args, NULL, NULL);

  if (__argc != 2) {
    _throw_type_error(_env, "Expected two arguments: path, callback");
    return NULL;
  }

  napi_value __path = __args[0];
  napi_value __callback = __args[1];

  // Get the address to bind to (if any)
  NAPI_CALL_VALUE(napi_typeof, _env, __path, &__type);
  if (__type != napi_string) {
    _throw_type_error(_env, "Path must be a string");
    return NULL;
  } else {
    // Convert the address string into a C string
    NAPI_CALL_VALUE(napi_get_value_string_utf8, _env, __path, __data.__path, PATH_MAX, NULL);
  }

  // Get the type of our first and only argument, must be a function
  NAPI_CALL_VALUE(napi_typeof, _env, __callback, &__type);

  if (__type != napi_function) {
    _throw_type_error(_env, "Specified callback is not a function");
    return NULL;
  }

  // Create a reference to our callback function
  NAPI_CALL_VALUE(napi_create_reference, _env, __callback, 1, &__data.__callback_ref);

  // Create a resource and resource name for our async work
  napi_value __resource = NULL;
  napi_value __resource_name = NULL;

  NAPI_CALL_VALUE(napi_create_object, _env, &__resource);
  NAPI_CALL_VALUE(napi_create_string_latin1, _env, "statvfs", NAPI_AUTO_LENGTH, &__resource_name);

  // Allocate _now_ the real `_statvfs_data` structure. We do it here because if
  // there are errors above we won't leak memory...
  struct _statvfs_data * __data_ptr = malloc(sizeof(struct _statvfs_data));
  memcpy(__data_ptr, &__data, sizeof(struct _statvfs_data));

  // Create our async work to be queued
  NAPI_CALL_VALUE(napi_create_async_work,
                  _env,
                  __resource,
                  __resource_name,
                  &_statvfs_execute,
                  &_statvfs_complete,
                  __data_ptr,
                  &__data_ptr->__async_work);

  // Queue up our async work
  NAPI_CALL_VALUE(napi_queue_async_work, _env, __data_ptr->__async_work);

  // Return JS `undefined`
  return NULL;
}

/* ========================================================================== *
 * init: initialize the addon, injecting our properties in the `exports`      *
 * ========================================================================== */
static napi_value init(napi_env _env, napi_value _exports) {
  napi_value __version = NULL;
  NAPI_CALL_VALUE(napi_create_string_latin1, _env, ADDON_VERSION, NAPI_AUTO_LENGTH, &__version);
  NAPI_CALL_VALUE(napi_set_named_property, _env, _exports, "version", __version);

  napi_value __statvfs_fn = NULL;
  NAPI_CALL_VALUE(napi_create_function, _env, "statvfs", NAPI_AUTO_LENGTH, _statvfs, NULL, &__statvfs_fn);
  NAPI_CALL_VALUE(napi_set_named_property, _env, _exports, "statvfs", __statvfs_fn);

  NAPI_CALL_VALUE(napi_object_freeze, _env, _exports);
  return _exports;
}

/* Define `init` as the entry point for our module */
NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
