#include "_build_metatables.h"
#include "_all.h"

namespace editor_script {
   void build_all_wrapper_metatables(lua_State* L) {
      using namespace wrappers;
      //
      define_wrapper_metatable<wrapper_metatable>(L);
      define_wrapper_metatable<form>(L);
      #pragma region Common form components
         define_wrapper_metatable<papyrus_root>(L);
         papyrus_root::build_collection_metatables(L);
            define_wrapper_metatable<papyrus_script>(L);
            papyrus_script::build_collection_metatables(L);
               define_wrapper_metatable<papyrus_property>(L);
               papyrus_property::build_collection_metatables(L);
      #pragma endregion
      #pragma region Form types
         define_wrapper_metatable<cell>(L);
            define_wrapper_metatable<cell_grid_coords>(L);
         define_wrapper_metatable<formlist>(L);
            formlist::build_collection_metatables(L);
         define_wrapper_metatable<landscape>(L);
            define_wrapper_metatable<landscape_quad_list>(L);
               define_wrapper_metatable<landscape_quad>(L);
                  define_wrapper_metatable<landscape_quad_alpha_layer>(L);
         define_wrapper_metatable<land_texture>(L);
            land_texture::build_collection_metatables(L);
            define_wrapper_metatable<land_texture_havok>(L);
         define_wrapper_metatable<quest>(L);
            quest::build_collection_metatables(L);
            define_wrapper_metatable<quest_alias>(L);
            define_wrapper_metatable<quest_loc_alias>(L);
            define_wrapper_metatable<quest_ref_alias>(L);
         define_wrapper_metatable<shout>(L);
            shout::build_collection_metatables(L);
            define_wrapper_metatable<shout_word>(L);
         define_wrapper_metatable<texture_set>(L);
            define_wrapper_metatable<texture_set_path_list>(L);
         define_wrapper_metatable<topic>(L);
         define_wrapper_metatable<topic_info>(L);
            topic_info::build_collection_metatables(L);
            define_wrapper_metatable<topic_info_response>(L);
         define_wrapper_metatable<voicetype>(L);
         define_wrapper_metatable<word_of_power>(L);
         define_wrapper_metatable<worldspace>(L);
            define_wrapper_metatable<worldspace_grid_bounds>(L);
               define_wrapper_metatable<worldspace_grid_bounds_extent>(L);
      #pragma endregion
      #pragma region Resources
         define_wrapper_metatable<resource::dds>(L);
         resource::dds::build_collection_metatables(L);
            define_wrapper_metatable<resource::dds_image_subresource>(L);
            resource::dds_image_subresource::build_collection_metatables(L);
               define_wrapper_metatable<resource::dds_cubemap_face_list>(L);
         define_wrapper_metatable<resource::raster>(L);
         define_wrapper_metatable<resource::unknown>(L);
      #pragma endregion
      #pragma region UI
         define_wrapper_metatable<ui::widget>(L);
            define_wrapper_metatable<ui::button>(L);
            define_wrapper_metatable<ui::canvas>(L);
               define_wrapper_metatable<ui::canvas_layer>(L);
               define_wrapper_metatable<ui::canvas_layer_group>(L);
               define_wrapper_metatable<ui::canvas_text_data>(L);
            define_wrapper_metatable<ui::checkbox>(L);
            define_wrapper_metatable<ui::dropdown>(L);
               define_wrapper_metatable<ui::dropdown_item>(L);
            define_wrapper_metatable<ui::formpicker>(L);
            define_wrapper_metatable<ui::groupbox>(L);
            define_wrapper_metatable<ui::image_widget>(L);
            define_wrapper_metatable<ui::line>(L);
            define_wrapper_metatable<ui::progress_bar>(L);
            define_wrapper_metatable<ui::radio_button>(L);
            define_wrapper_metatable<ui::radio_group>(L);
            define_wrapper_metatable<ui::scrollbox>(L);
            define_wrapper_metatable<ui::spinbox>(L);
            define_wrapper_metatable<ui::tabbox>(L);
               define_wrapper_metatable<ui::tabbox_tab>(L);
            define_wrapper_metatable<ui::table_view>(L);
               define_wrapper_metatable<ui::table_view_row>(L);
               define_wrapper_metatable<ui::table_view_col>(L);
               define_wrapper_metatable<ui::table_view_cell>(L);
            define_wrapper_metatable<ui::text>(L);
            define_wrapper_metatable<ui::textbox>(L);
            define_wrapper_metatable<ui::window>(L);
         #pragma region Various
            define_wrapper_metatable<ui::font>(L);
         #pragma endregion
      #pragma endregion
      define_wrapper_metatable<raster_draw_path>(L);
   }
}