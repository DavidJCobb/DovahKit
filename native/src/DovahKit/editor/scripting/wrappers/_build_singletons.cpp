#include "_build_singletons.h"
#include "_all_resources.h"
#include "_all_ui.h"

namespace editor_script {
   void build_all_resource_wrapper_singletons(lua_State* L) {
      using namespace wrappers;
      //
      resource::raster::setup(L);
      // NOTE: As of this writing the "unknown" type DOES NOT have a singleton
   }
   void build_all_ui_wrapper_singletons(lua_State* L) {
      using namespace wrappers;
      //
      ui::button::setup(L);
      ui::canvas::setup(L);
      ui::checkbox::setup(L);
      ui::dropdown::setup(L);
         ui::dropdown_item::setup(L);
      ui::formpicker::setup(L);
      ui::groupbox::setup(L);
      ui::image_widget::setup(L);
      ui::line::setup(L);
      ui::progress_bar::setup(L);
      ui::radio_button::setup(L);
      ui::radio_group::setup(L);
      ui::scrollbox::setup(L);
      ui::spinbox::setup(L);
      ui::tabbox::setup(L);
         ui::tabbox_tab::setup(L);
      ui::table_view::setup(L);
         ui::table_view_row::setup(L);
         ui::table_view_col::setup(L);
         ui::table_view_cell::setup(L);
      ui::text::setup(L);
      ui::textbox::setup(L);
      ui::widget::setup(L);
      ui::window::setup(L);
   }
}