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
            define_wrapper_metatable<papyrus_root>(L);
               define_wrapper_metatable<papyrus_script>(L);
                  define_wrapper_metatable<papyrus_property>(L);
         #pragma endregion
         #pragma region Form types
            define_wrapper_metatable<quest>(L);
               define_wrapper_metatable<quest_alias>(L);
                  define_wrapper_metatable<quest_loc_alias>(L);
                  define_wrapper_metatable<quest_ref_alias>(L);
         #pragma endregion
      #pragma endregion
      #pragma region Resources
         set_up_class_with_global_singleton<resource::dds>(L);
            define_wrapper_metatable<resource::dds_cubemap_face_list>(L);
            define_wrapper_metatable<resource::dds_image_subresource>(L);
         set_up_class_with_global_singleton<resource::raster>(L);
         set_up_class_with_global_singleton<resource::unknown>(L);
      #pragma endregion
      #pragma region UI
      {
         lua_createtable(L, 0, 12);
         lua_pushvalue  (L, -1);
         lua_setglobal  (L, "ui");
         auto index = lua_gettop(L);
         //
         set_up_class_with_singleton<ui::widget>(L, index);
            set_up_class_with_singleton<ui::table_view>(L, index);
               set_up_class_with_singleton<ui::table_view_cell>(L, index);
               set_up_class_with_singleton<ui::table_view_col>(L, index);
               set_up_class_with_singleton<ui::table_view_row>(L, index);
            set_up_class_with_singleton<ui::window>(L, index);
         #pragma region Various
         define_wrapper_metatable<ui::font>(L);
         #pragma endregion
         //
         assert(lua_gettop(L) == index);
         lua_pop(L, 1);
      }
      #pragma endregion
   }
}