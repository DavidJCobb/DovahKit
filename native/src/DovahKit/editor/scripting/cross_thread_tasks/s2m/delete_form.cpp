#include "delete_form.h"
#include "../../../core.h"
#include "../../../../dovah/files/file_load_order.h"
#include "../../../../dovah/notice_code_list.h"

namespace {
   const char* _explain_error_code(dovah::notice_code_t code) {
      using notice_code = dovah::notice_code;
      switch (code) {
         case notice_code::no_active_file:
            return "cannot delete the form because there is neither an active file nor any room in the load order for a new file";
         case notice_code::unimplemented_form_type:
            return "cannot delete the form because DovahKit doesn't yet know how to load forms of this type";
         case notice_code::cannot_load_all_users_of_this_form:
            return "cannot delete the form because one of the forms that uses it is of a type that DovahKit doesn't yet know how to load";
         case notice_code::cannot_delete_hardcoded_form:
            return "cannot delete the form because it is hardcoded";
      }
      return "cannot delete the form for an unknown reason";
   }
}

namespace editor_script::tasks::s2m {
   /*virtual*/ void delete_form::_exec_impl() /*override*/ {
      assert(this->stub);
      auto& editor = DovahKitCore::get();
      editor.delete_form(*this->stub,
         [this](const dovah::form_deletion_request& request) {
            auto error = request.get_error_code();
            if (error != dovah::default_notice_code) {
               this->error      = true;
               this->error_text = _explain_error_code(error);
               return false;
            }
            return true;
         },
         [](const dovah::form_deletion_request& request) {}
      );
   }
}