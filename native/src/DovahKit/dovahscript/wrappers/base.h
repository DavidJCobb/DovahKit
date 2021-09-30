#pragma once
#include <cassert>
#include <initializer_list>
#include "../../lua.h"
#include "../core/classes.h"

namespace dovahscript {
   struct wrapper_metatable {
      //
      // Struct for defining metatable names and methods at compile-time.
      //
      protected:
         using string_list_t = const std::initializer_list<const char*>;
         using method_list_t = const std::initializer_list<luaL_Reg>;
      private:
         wrapper_metatable() = delete;
      public:
         static constexpr string_list_t superclass_list = {}; // direct subclasses must set this to { metatable_key } before defining their own, so as to inherit common GC metamethods
         static constexpr const char*   metatable_key   = "dovah.classes.!base"; // subclasses must override this
         static constexpr const char*   class_name      = nullptr;               // subclasses should override this
         static method_list_t metatable_methods; // subclasses must override this even if they offer no methods
         static method_list_t metatable_getters; // subclasses must override this even if they offer no getters
         static method_list_t metatable_setters; // subclasses must override this even if they offer no setters

         static constexpr bool has_extra_class_setup = false;
         static void extra_class_setup(lua_State* L) {
            // Subclasses can override this, but it will only be called if (has_extra_class_setup) is (true).
            // Index -3 is the class.
            // Index -2 is the getter list.
            // Index -1 is the setter list.
         };

         static constexpr std::initializer_list<luaL_Reg> no_functions = {};
   };
   
   template<typename T> void define_wrapper_metatable(lua_State* L) noexcept {
      if (!T::metatable_key)
         return;
      if (classes::is_class_defined(L, T::metatable_key))
         return;
      classes::define_class(L, T::metatable_key, T::superclass_list, T::metatable_methods, T::metatable_getters, T::metatable_setters, T::class_name);
      if (T::has_extra_class_setup) {
         auto got = classes::get_class_tables(L, T::metatable_key);
         assert(got && "How did this fail? The class should have been defined successfully!");
         auto top = lua_gettop(L);
         T::extra_class_setup(L);
         assert(lua_gettop(L) >= top);
         lua_settop(L, top - got);
      }
   }
}