#include "./benchmark.h"
#include "helpers/lua/error.h"
#include "helpers/windows.h"
#include "../core/classes.h"

namespace {
   LARGE_INTEGER _get_frequency() {
      LARGE_INTEGER temp;
      if (!QueryPerformanceFrequency(&temp))
         temp.QuadPart = 0;
      return temp;
   }

   class benchmark_native {
      public:
         benchmark_native() {
            QueryPerformanceCounter(&this->time);
         }

         LARGE_INTEGER time;
         bool done = false;

         void start() {
            this->done = false;
            QueryPerformanceCounter(&this->time);
         }
         void finish() {
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

         inline uint32_t microseconds() const noexcept {
            return this->time.QuadPart;
         }
         inline uint32_t milliseconds() const noexcept {
            return this->time.QuadPart / 1000;
         }
   };
}

namespace {
   using namespace dovahscript;
   using cls          = lua_classes::benchmark;
   using wrapped_type = benchmark_native;

   wrapped_type& _get_this(lua_State* L) {
      auto* self = (wrapped_type*) classes::cast_to_exact_class(L, 1, cls::metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", cls::metatable_key);
      return *self;
   }

   namespace _methods {
      int restart(lua_State* L) {
         _get_this(L).start();
         return 0;
      }
      int stop(lua_State* L) {
         auto& self = _get_this(L);
         if (self.done)
            return 0;
         self.finish();
         return 0;
      }
      int time_to_string(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done) {
            lua_pushstring(L, "still running");
            return 1;
         }
         lua_pushfstring(L, (const char*)u8"%d ms / %d \u03BCs", self.milliseconds(), self.microseconds());
         return 1;
      }
      int __tostring(lua_State* L) {
         auto& self = _get_this(L);
         if (!self.done) {
            lua_pushstring(L, "benchmark (running)");
            return 0;
         }
         lua_pushfstring(L, (const char*)u8"benchmark (%u ms / %u \u03BCs)", self.milliseconds(), self.microseconds());
         return 1;
      }
   }
   namespace _getters {
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
   }

   namespace _singleton_functions {
      int new_obj(lua_State* L) {
         auto* p = lua_newuserdata(L, sizeof(wrapped_type)); // push 1
         luaL_getmetatable(L, cls::metatable_key); // push 1
         lua_setmetatable(L, -2); // pop 1
         new (p) wrapped_type;
         return 1;
      }
      int is(lua_State* L) {
         auto* arg = (cls*)classes::cast_to_exact_class(L, 1, cls::metatable_key);
         lua_pushboolean(L, arg != nullptr);
         return 1;
      }
   }
}
namespace dovahscript::lua_classes {
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "restart",        &_methods::restart },
      { "stop",           &_methods::stop },
      { "time_to_string", &_methods::time_to_string },
      { "__tostring",     &_methods::__tostring },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "microseconds", &_getters::microseconds },
      { "milliseconds", &_getters::milliseconds },
   };
   /*static*/ std::initializer_list<luaL_Reg> cls::metatable_setters = {
   };

   /*static*/ void cls::push_new_instance(lua_State* L) {
      auto* p = lua_newuserdata(L, sizeof(cls)); // push 1
      luaL_getmetatable(L, cls::metatable_key); // push 1
      lua_setmetatable(L, -2); // pop 1
      new (p) cls;
   }
   /*static*/ void cls::setup(lua_State* L) {
      classes::define_class(L, metatable_key, nullptr, metatable_methods, metatable_getters, metatable_setters);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_obj);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      lua_setglobal(L, cls::global_name);
   }
}