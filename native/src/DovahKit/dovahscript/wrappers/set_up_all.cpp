#include "set_up_all.h"
#include "../../lua.h"

#include "form/_all.h"
#include "resource/_all.h"
#include "ui/_all.h"
#include "unusual/_all.h"
#include "ini/setting.h"

namespace {
   template<typename T> concept _HasSingletonToImport = requires (lua_State* L) {
      T::import_singleton(L);
      std::is_same_v<const char*, decltype(T::global_name)>;
   };

   //
   // Sets up a widget class's metatable, and calls its static (setup) member function 
   // to create its singleton.
   //
   template<typename T> requires _HasSingletonToImport<T>
   inline void set_up_class_with_singleton(lua_State* L, int pos) {
      dovahscript::define_wrapper_metatable<T>(L);
      T::import_singleton(L);
      lua_setfield(L, pos, T::global_name);
      assert(lua_gettop(L) == pos);
   }

   template<typename T> requires _HasSingletonToImport<T>
   inline void set_up_class_with_global_singleton(lua_State* L) {
      dovahscript::define_wrapper_metatable<T>(L);
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
            define_wrapper_metatable<cell>(L);
               define_wrapper_metatable<cell_grid_coords>(L);
            define_wrapper_metatable<formlist>(L);
            define_wrapper_metatable<land_texture>(L);
               define_wrapper_metatable<land_texture_havok>(L);
            define_wrapper_metatable<landscape>(L);
               define_wrapper_metatable<landscape_quad>(L);
               define_wrapper_metatable<landscape_quad_alpha_layer>(L);
               define_wrapper_metatable<landscape_quad_list>(L);
            define_wrapper_metatable<quest>(L);
               define_wrapper_metatable<quest_alias>(L);
                  define_wrapper_metatable<quest_loc_alias>(L);
                  define_wrapper_metatable<quest_ref_alias>(L);
            define_wrapper_metatable<shout>(L);
               define_wrapper_metatable<shout_word>(L);
            define_wrapper_metatable<texture_set>(L);
               define_wrapper_metatable<texture_set_path_list>(L);
            define_wrapper_metatable<topic>(L);
            define_wrapper_metatable<topic_info>(L);
               define_wrapper_metatable<topic_info_response>(L);
            define_wrapper_metatable<voicetype>(L);
            define_wrapper_metatable<word_of_power>(L);
            define_wrapper_metatable<worldspace>(L);
               define_wrapper_metatable<worldspace_grid_bounds>(L);
               define_wrapper_metatable<worldspace_grid_bounds_extent>(L);
         #pragma endregion
      #pragma endregion
      #pragma region INI
         define_wrapper_metatable<ini::setting>(L);
      #pragma endregion
      #pragma region Resources
         set_up_class_with_global_singleton<resource::binary_view>(L);
         define_wrapper_metatable<resource::dds>(L);
            define_wrapper_metatable<resource::dds_cubemap_face_list>(L);
            define_wrapper_metatable<resource::dds_image_subresource>(L);
         set_up_class_with_global_singleton<resource::raster>(L);
         define_wrapper_metatable<resource::unknown>(L);
      #pragma endregion
      #pragma region UI
      {
         auto t = lua_getglobal(L, "ui");
         if (t == LUA_TNONE || t == LUA_TNIL) {
            lua_createtable(L, 0, 20);
            lua_pushvalue(L, -1);
            lua_setglobal(L, "ui");
         } else {
            assert(t == LUA_TTABLE && "How is the `ui` global not a table?!");
         }
         auto index = lua_gettop(L);
         //
         set_up_class_with_singleton<ui::widget>(L, index);
            set_up_class_with_singleton<ui::button>(L, index);
            set_up_class_with_singleton<ui::canvas>(L, index);
               define_wrapper_metatable<ui::canvas_layer>(L);
               define_wrapper_metatable<ui::canvas_layer_group>(L);
               set_up_class_with_singleton<ui::canvas_text_data>(L, index);
            set_up_class_with_singleton<ui::checkbox>(L, index);
            set_up_class_with_singleton<ui::color_button>(L, index);
            set_up_class_with_singleton<ui::dropdown>(L, index);
               set_up_class_with_singleton<ui::dropdown_item>(L, index);
            set_up_class_with_singleton<ui::formpicker>(L, index);
            set_up_class_with_singleton<ui::groupbox>(L, index);
            set_up_class_with_singleton<ui::image_widget>(L, index);
            set_up_class_with_singleton<ui::line>(L, index);
            set_up_class_with_singleton<ui::progress_bar>(L, index);
            set_up_class_with_singleton<ui::radio_button>(L, index);
            set_up_class_with_singleton<ui::radio_group>(L, index);
            set_up_class_with_singleton<ui::scrollbox>(L, index);
            set_up_class_with_singleton<ui::spinbox>(L, index);
            set_up_class_with_singleton<ui::tabbox>(L, index);
               set_up_class_with_singleton<ui::tabbox_tab>(L, index);
            set_up_class_with_singleton<ui::table_view>(L, index);
               set_up_class_with_singleton<ui::table_view_cell>(L, index);
               set_up_class_with_singleton<ui::table_view_col>(L, index);
               set_up_class_with_singleton<ui::table_view_row>(L, index);
            set_up_class_with_singleton<ui::text>(L, index);
            set_up_class_with_singleton<ui::textarea>(L, index);
            set_up_class_with_singleton<ui::textbox>(L, index);
            set_up_class_with_singleton<ui::window>(L, index);
         //
         #pragma region Various
            define_wrapper_metatable<ui::font>(L);
         #pragma endregion
         //
         assert(lua_gettop(L) == index);
         lua_pop(L, 1);
      }
      #pragma endregion
      #pragma region Unusual
         set_up_class_with_global_singleton<raster_draw_path>(L);
      #pragma endregion
   }
}