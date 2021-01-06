#include "delete_form.h"
#include "../../editor_script_core.h"
#include "../../../core.h"
#include "../../../../dovah/files/file_load_order.h"

namespace editor_script::tasks::s2m {
   /*virtual*/ void delete_form::_exec_impl() /*override*/ {
      assert(this->stub);
      auto& editor = DovahKitCore::get();
      editor.delete_form(*this->stub,
         [this](const dovah::form_deletion_request& request) {
            auto error = request.get_result_code();
            if (error != decltype(error)::pending) {
               using code_t = decltype(error);
               //
               this->error = true;
               switch (error) {
                  case code_t::error_cannot_delete_hardcoded_form:
                     this->error_text = "cannot delete the form because it is hardcoded";
                     break;
                  case code_t::error_cannot_load_form:
                     this->error_text = "cannot delete the form because DovahKit doesn't yet know how to load forms of this type";
                     break;
                  case code_t::error_cannot_load_user:
                     this->error_text = "cannot delete the form because one of the forms that uses it is of a type that DovahKit doesn't yet know how to load";
                     break;
                  default:
                     this->error_text = "cannot delete the form for an unknown reason";
                     break;
               }
               return false;
            }
            //
            auto forms = request.get_forms_pending_delete(false); // only include forms that we will erase from memory, not simply ones we'll slap the "deleted" flag on
            for (auto* stub : forms)
               DovahKitScriptVMUserdataInterface::get().remove_form(*stub);
            return true;
         },
         [](const dovah::form_deletion_request& request) {}
      );
   }
}