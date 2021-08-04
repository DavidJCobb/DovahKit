#include "set_up_all.h"
#include "../../lua.h"

#include "form/_all.h"
#include "resource/_all.h"
#include "ui/_all.h"

namespace {
   //
   // Sets up a widget class's metatable, and calls its static (setup) member function 
   // to create its singleton.
   //
   template<typename T> inline void set_up_class_with_singleton(lua_State* L, int pos) {
      define_wrapper_metatable<T>(L);
      T::import_singleton(L);
      lua_setfield(L, pos, cls::global_name);
      assert(lua_gettop(L) == pos);
   }
   template<typename T> inline void set_up_class_with_global_singleton(lua_State* L) {
      define_wrapper_metatable<T>(L);
      T::import_singleton(L);
      lua_setglobal(L, T::global_name);
   }
}

namespace dovahscript {
   extern void set_up_all_native_wrappers(lua_State* L) {
      using namespace wrappers;
      //
      define_wrapper_metatable<wrapper_metatable>(L);
      #pragma region Form wrappers
         define_wrapper_metatable<form>(L);
         #pragma region Common form components
         #pragma endregion
         #pragma region Form types
         #pragma endregion
      #pragma endregion
      #pragma region Resources
         // set_up_class_with_global_singleton<some_wrapper>(L);
      #pragma endregion
      #pragma region UI
      {
         lua_createtable(L, 0, 12);
         lua_pushvalue  (L, -1);
         lua_setglobal  (L, "ui");
         auto index = lua_gettop(L);
         //
         set_up_class_with_singleton<ui::widget>(L, index);
         #pragma region Various
         #pragma endregion
         //
         assert(lua_gettop(L) == index);
         lua_pop(L, 1);
      }
      #pragma endregion
   }
}