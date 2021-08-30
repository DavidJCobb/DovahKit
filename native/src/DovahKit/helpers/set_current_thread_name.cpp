#include "set_current_thread_name.h"
#include <windows.h>
#include <processthreadsapi.h>

namespace cobb {
   extern void set_current_thread_name(const wchar_t* name) {
      SetThreadDescription(GetCurrentThread(), name);
   }
}