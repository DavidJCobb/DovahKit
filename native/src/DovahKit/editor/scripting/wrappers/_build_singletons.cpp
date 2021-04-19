#include "_build_singletons.h"
#include "_all_ui.h"

namespace editor_script {
   void build_all_ui_wrapper_singletons(lua_State* L) {
      using namespace wrappers;
      //
      ui::button::setup(L);
      ui::checkbox::setup(L);
      ui::dropdown::setup(L);
         ui::dropdown_item::setup(L);
      ui::formpicker::setup(L);
      ui::groupbox::setup(L);
      ui::line::setup(L);
      ui::progress_bar::setup(L);
      ui::radio_button::setup(L);
      ui::radio_group::setup(L);
      ui::spinbox::setup(L);
      ui::table_view::setup(L);
      ui::text::setup(L);
      ui::textbox::setup(L);
      ui::widget::setup(L);
      ui::window::setup(L);
   }
}