#pragma once
#include <napi.h>

namespace Errno {
  Napi::Error ErrnoError(
    Napi::Env env,
    int e,
    const char *syscall = nullptr,
    const char *path = nullptr,
    const char *message = nullptr
  );
}
