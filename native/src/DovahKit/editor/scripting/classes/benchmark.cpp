#include "benchmark.h"
#include "../util.h"
#include "../../../helpers/lua/error.h"

namespace {
   LARGE_INTEGER _get_frequency() {
      LARGE_INTEGER temp;
      if (!QueryPerformanceFrequency(&temp))
         temp.QuadPart = 0;
      return temp;
   }
}

namespace editor_script::classes {
   benchmark::benchmark() {
      QueryPerformanceCounter(&this->time);
   }
   void benchmark::finish() {
      if (this->done)
         return;
      LARGE_INTEGER end;
      QueryPerformanceCounter(&end);
      //
      static LARGE_INTEGER frequency = _get_frequency();
      //
      this->time.QuadPart = end.QuadPart - this->time.QuadPart;
      this->done = true;
      if (!frequency.QuadPart) {
         this->time.QuadPart = 0;
         return;
      }
      //
      // to microseconds:
      //
      this->time.QuadPart *= 1000000;
      this->time.QuadPart /= frequency.QuadPart;
   }
}

namespace {
   using namespace editor_script;

   classes::benchmark& _get_this(lua_State* L) {
      auto* self = (classes::benchmark*) editor_script::cast_to_exact_class(L, 1, classes::benchmark::metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", classes::benchmark::metatable_key);
      return *self;
   }

   namespace _methods {
      luastackchange_t microseconds(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done)
            return 0;
         lua_pushinteger(L, self.microseconds());
         return 1;
      }
      luastackchange_t milliseconds(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done)
            return 0;
         lua_pushinteger(L, self.milliseconds());
         return 1;
      }
   }
}
namespace editor_script::classes {
   /*static*/ std::initializer_list<luaL_Reg> benchmark::metatable_methods = {
      { "microseconds", &_methods::microseconds },
      { "milliseconds", &_methods::milliseconds },
   };
}