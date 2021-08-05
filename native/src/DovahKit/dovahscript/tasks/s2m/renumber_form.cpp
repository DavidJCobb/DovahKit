#include "renumber_form.h"
#include "../../../editor/core.h"
#include "../../../dovah/files/file_load_order.h"
#include "../../../dovah/notice_code_list.h"

namespace {
   const char* _explain_error_code(dovah::notice_code_t code) {
      using notice_code = dovah::notice_code;
      switch (code) {
         case notice_code::form_is_not_defined_in_active_file:
            return "cannot renumber the form because it doesn't originate from the active file";
         case notice_code::cannot_renumber_hardcoded_form:
            return "cannot renumber the form because it is hardcoded";
         case notice_code::form_id_is_in_the_hardcoded_range:
            return "cannot renumber the form because the requested form ID is in the hardcoded range";
         case notice_code::zero_is_not_an_allowed_form_id:
            return "cannot renumber the form because a form cannot use zero as its form ID";
         case notice_code::form_id_is_out_of_bounds:
            return "cannot renumber the form because the requested form ID is out-of-bounds (the load order prefix places it outside of all loaded files)";
         case notice_code::form_id_is_already_in_use:
            return "cannot renumber the form because the requested form ID is already in use";
         case notice_code::form_id_is_reserved_for_other_process:
            return "cannot renumber the form because the DovahKit has reserved the requested form ID for use by some other process";
         case notice_code::cannot_load_all_users_of_this_form:
            return "cannot renumber the form because one of the forms that uses it is of a type that DovahKit doesn't yet know how to load";
         case notice_code::no_active_file:
            return "cannot renumber the form because there is neither an active file nor any room in the load order for a new file";
         case notice_code::cannot_sever_references_to_none_stub:
            return "cannot renumber the form because the requested form ID is the target of one or more dangling references, and at least one such reference is outbound from a form of a type that DovahKit doesn't yet know how to load";
         case notice_code::cannot_inject_form_overtop_none_stub:
            return "cannot renumber the form because the requested form ID is the target of one or more dangling references, and at least one such reference is outbound from a form not defined in the active file";
      }
      return "cannot renumber the form for an unknown reason";
   }
}

namespace dovahscript::tasks::s2m {
   /*virtual*/ void renumber_form::_exec_impl() /*override*/ {
      assert(this->stub);
      auto& editor  = DovahKitCore::get();
      auto  request = editor.request_form_renumber(*this->stub, this->desiredID);
      //
      auto code = request.get_error_code();
      if (code != dovah::default_notice_code) {
         this->error      = true;
         this->error_text = _explain_error_code(code);
         return;
      }
      //
      request.commit();
      //
      code = request.get_error_code();
      if (code != dovah::default_notice_code) {
         this->error      = true;
         this->error_text = _explain_error_code(code);
      }
   }
}