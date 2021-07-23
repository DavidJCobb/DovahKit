#pragma once
#include <initializer_list>
#include "../../lua.h"

namespace dovahscript {
   struct wrapper_metatable {
      //
      // Struct for defining metatable names and methods at compile-time.
      //
      private:
         wrapper_metatable() = delete;
      public:
         static constexpr const char* superclass_key = nullptr;               // direct subclasses should set this to (metatable_key) before defining their own, so as to inherit common GC metamethods
         static constexpr const char* metatable_key  = "dovah.classes.!base"; // subclasses must override this
         static constexpr const char* class_name     = nullptr;               // subclasses should override this
         static const std::initializer_list<luaL_Reg> metatable_methods; // subclasses must override this even if they offer no methods
         static const std::initializer_list<luaL_Reg> metatable_getters; // subclasses must override this even if they offer no getters
         static const std::initializer_list<luaL_Reg> metatable_setters; // subclasses must override this even if they offer no setters

         static constexpr bool has_extra_class_setup = false;
         static void extra_class_setup(lua_State* L) noexcept {
            // Subclasses can override this, but it will only be called if (has_extra_setup) is 
            // (true).
         };

         static constexpr std::initializer_list<luaL_Reg> no_functions = {};
   };
   
   template<typename T> void define_wrapper_metatable(lua_State* L) noexcept {
      if (!T::metatable_key)
         return;
      if (is_class_defined(L, T::metatable_key))
         return;
      define_class(L, T::metatable_key, T::superclass_key, T::metatable_methods, T::metatable_getters, T::metatable_setters, T::class_name);
      if (T::has_extra_class_setup) {
         auto got = get_class_tables(L, T::metatable_key);
         assert(got && "How did this fail? The class should have been defined successfully!");
         auto top = lua_gettop(L);
         T::extra_class_setup(L);
         assert(lua_gettop(L) >= top);
         lua_settop(L, top - got);
      }
   }
}