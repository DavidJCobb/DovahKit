#include "./delete_form.h"
#include "editor/core.h"
#include "dovah/exceptions/form_deletion_failed.h"
#include "dovah/files/file_load_order.h"
#include "../../core/verify_threading.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/userdata.h"

namespace {
   using exception  = dovah::exceptions::form_deletion_failed;
   using error_code = exception::error_code;

   const char* _explain_error_code(error_code code) {
      switch (code) {
         case error_code::wrong_load_order:
            return "cannot delete the form because something impossible happened (wrong_load_order); contact Dovahit's developer";

         case error_code::no_active_file:
            return "cannot delete the form because there is neither an active file nor any room in the load order for a new file";
         case error_code::unimplemented_form_type:
            return "cannot delete the form because DovahKit doesn't yet know how to load forms of this type";
         case error_code::cannot_load_all_users_of_this_form:
            return "cannot delete the form because one of the forms that uses it is of a type that DovahKit doesn't yet know how to load";
         case error_code::form_is_hardcoded:
            return "cannot delete the form because it is hardcoded";
      }
      return "cannot delete the form for an unknown reason";
   }

   using coordinator_passkey = dovahscript::core::subsystems::coordinator::passkey_to<dovahscript::tasks::s2m::delete_form>;
}

namespace dovahscript::tasks::s2m {
   delete_form::delete_form() {
      //
      // We must run Lua code in order to zombify the userdata wrappers for all forms 
      // to be deleted just before they're deleted. We can't zombify wrappers after 
      // the forms are deleted, because we need to ensure that the wrappers clear out 
      // their loaded_form_ptrs prior to deletion.
      //
      this->_needs_lua_ownership = true;
   }

   void delete_form::run_lua_before(lua_State* L) {
      this->lua_state = L;
   }

   void delete_form::_exec_impl() {
      core::require_client_thread();
      core::require_script_thread();
      //
      assert(this->stub);
      assert(this->lua_state);
      auto& editor = DovahKitCore::get();
      auto* L      = this->lua_state;
      editor.delete_form(
         *this->stub,
         //
         // Lambda executed after all forms to be deleted are known; the last chance to 
         // cancel the deletion, or to handle errors if any occurred. Return true to 
         // proceed with deletion, or false to cancel.
         //
         [this, L](const dovah::form_deletion_request& request) {
            auto& userdata_s = core::subsystems::userdata::get();
            auto  list       = request.get_forms_pending_delete(false);
            for (auto* stub : list)
               userdata_s.destroy_all(*stub);
            core::subsystems::coordinator::get().expect_deletion_of(coordinator_passkey(), list);
            return true;
         },
         [this, L](const dovah::exceptions::form_deletion_failed& ex) {
            this->results.failed = true;
            this->results.text   = _explain_error_code(ex.code);
         },
         //
         // Lambda executed after the "deletion imminent" signals were emitted, after 
         // deletion has occurred, but before "deletion complete" signals were emitted.
         //
         [](const dovah::form_deletion_request& request) {
            core::subsystems::coordinator::get().on_deletion_completion_expected(coordinator_passkey());
         }
      );
   }
}