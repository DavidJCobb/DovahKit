#include "test_call_and_response.h"
#include "../../editor_script_core.h"
#include <QMessageBox>

namespace editor_script::tasks::s2m {
   /*virtual*/ void test_call_and_response::_exec_impl() /*override*/ {
      auto& vm = DovahKitScriptVM::get();
      QMessageBox::information(vm.get_ui_parent_widget(), "Test", "This should block script execution until it is dismissed");
   }
}