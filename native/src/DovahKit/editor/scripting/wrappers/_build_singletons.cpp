#include "_build_singletons.h"
#include "_all_ui.h"

namespace editor_script {
   void build_all_ui_wrapper_singletons(lua_State* L) {
      using namespace wrappers;
      //
      ui::window::setup(L);
   }
}