#include "permissions.h"
#include "../../../../Lua/lua.hpp"
#include "editor_script_inner_core.h"

/*static*/ void DovahKitScriptVMPermissionInterface::verify_form_write_permissions() {
   DovahKitScriptVMCore::require_script_thread();
   auto& intfc = DovahKitScriptVMPermissionInterface::get();
   if (false) { // TODO: permission check, when we implement those
      luaL_error(intfc.vm.lua_vm, "The script does not have permission to use APIs that modify form data.");
      __assume(0);
   }
}
/*static*/ void DovahKitScriptVMPermissionInterface::verify_ui_permissions() {
   DovahKitScriptVMCore::require_script_thread();
   auto& intfc = DovahKitScriptVMPermissionInterface::get();
   if (false) { // TODO: permission check, when we implement those
      luaL_error(intfc.vm.lua_vm, "The script does not have permission to use APIs related to the UI.");
      __assume(0);
   }
}
/*static*/ bool DovahKitScriptVMPermissionInterface::check_ui_html_permissions() {
   DovahKitScriptVMCore::require_script_thread();
   return true; // TODO: permission check, when we implement those
}