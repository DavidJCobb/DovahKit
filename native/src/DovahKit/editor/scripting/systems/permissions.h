#pragma once
#include "../../../helpers/singleton.h"
#include "editor_script_inner_core.h"

class DovahKitScriptVMPermissionInterface : cobb::singleton {
   protected:
      DovahKitScriptVMPermissionInterface(DovahKitScriptVMCore& w) : vm(w) {}
   public:
      static DovahKitScriptVMPermissionInterface& get() {
         static DovahKitScriptVMPermissionInterface instance(DovahKitScriptVMCore::get());
         return instance;
      }
      //
      DovahKitScriptVMCore& vm;

      static void verify_form_write_permissions();
      static void verify_ui_permissions();

      static bool check_ui_html_permissions();
};