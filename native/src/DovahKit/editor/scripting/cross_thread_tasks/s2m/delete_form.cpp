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
            //
            // TODO: if there are errors, store error information on the message.
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