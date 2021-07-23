#include "permissions.h"
#include "coordinator.h"
#include "../../../helpers/lua/error.h"

namespace dovahscript::core::subsystems {
   /*static*/ void permissions::verify_form_write_permissions() {
      coordinator::require_script_thread();
      auto& intfc = coordinator::get();
      if (false) { // TODO: permission check, when we implement those
         cobb::lua::error(intfc.vm.lua_vm, "The script does not have permission to use APIs that modify form data.");
      }
   }
   /*static*/ void permissions::verify_ui_permissions() {
      coordinator::require_script_thread();
      auto& intfc = coordinator::get();
      if (false) { // TODO: permission check, when we implement those
         cobb::lua::error(intfc.vm.lua_vm, "The script does not have permission to use APIs related to the UI.");
      }
   }
   /*static*/ bool permissions::check_ui_html_permissions() {
      coordinator::require_script_thread();
      return true; // TODO: permission check, when we implement those
   }
}