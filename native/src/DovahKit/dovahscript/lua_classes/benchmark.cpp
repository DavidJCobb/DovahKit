#include "benchmark.h"
#include "../../../helpers/lua/error.h"
#include "../core/classes.h"

namespace {
   using namespace dovahscript;
   using cls = dovahscript::lua_classes::benchmark;
}

namespace {
   LARGE_INTEGER _get_frequency() {
      LARGE_INTEGER temp;
      if (!QueryPerformanceFrequency(&temp))
         temp.QuadPart = 0;
      return temp;
   }
}

namespace dovahscript::lua_classes {
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
   cls& _get_this(lua_State* L) {
      auto* self = (cls*) classes::cast_to_exact_class(L, 1, cls::metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", cls::metatable_key);
      return *self;
   }

   namespace _methods {
      int microseconds(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done)
            return 0;
         lua_pushinteger(L, self.microseconds());
         return 1;
      }
      int milliseconds(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done)
            return 0;
         lua_pushinteger(L, self.milliseconds());
         return 1;
      }
      int stop(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done)
            return 0;
         self.finish();
         return 0;
      }
   }
}
namespace dovahscript::lua_classes {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "microseconds", &_methods::microseconds },
      { "milliseconds", &_methods::milliseconds },
      { "stop",         &_methods::stop },
   };

   /*static*/ void cls::push_new_instance(lua_State* L) {
      auto* p = lua_newuserdata(L, sizeof(cls)); // push 1
      luaL_getmetatable(L, cls::metatable_key); // push 1
      lua_setmetatable(L, -2); // pop 1
      new (p) cls;
   }
   /*static*/ void cls::setup(lua_State* L) {
      classes::define_class(L, metatable_key, nullptr, metatable_methods);
   }
}