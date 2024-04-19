#include "./renumber_form.h"
#include "dovah/exceptions/form_renumber_failed.h"
#include "dovah/files/file_load_order.h"
#include "editor/core.h"

namespace {
   using exception  = dovah::exceptions::form_renumber_failed;
   using error_code = exception::error_code;

   const char* _explain_error_code(error_code code) {
      switch (code) {
         case error_code::form_is_not_from_active_file:
            return "cannot renumber the form because it doesn't originate from the active file";
         case error_code::form_is_hardcoded:
            return "cannot renumber the form because it is hardcoded";
         case error_code::form_id_is_in_hardcoded_range:
            return "cannot renumber the form because the requested form ID is in the hardcoded range";
         case error_code::form_id_is_zero:
            return "cannot renumber the form because a form cannot use zero as its form ID";
         case error_code::form_id_is_out_of_bounds:
            return "cannot renumber the form because the requested form ID is out-of-bounds (the load order prefix places it outside of all loaded files)";
         case error_code::form_id_is_occupied:
            return "cannot renumber the form because the requested form ID is already in use";
         case error_code::form_id_is_reserved:
            return "cannot renumber the form because the DovahKit has reserved the requested form ID for use by some other process";
         case error_code::no_active_file:
            return "cannot renumber the form because there is neither an active file nor any room in the load order for a new file";
         case error_code::cannot_sever_references_to_none_stub:
            return "cannot renumber the form because the requested form ID is the target of one or more dangling references, and at least one such reference is outbound from a form of a type that DovahKit doesn't yet know how to load";
         case error_code::cannot_inject_form_overtop_none_stub:
            return "cannot renumber the form because the requested form ID is the target of one or more dangling references, and at least one such reference is outbound from a form not defined in the active file";
      }
      return "cannot renumber the form for an unknown reason";
   }
}

namespace dovahscript::tasks::s2m {
   /*virtual*/ void renumber_form::_exec_impl() /*override*/ {
      assert(this->stub);
      auto& editor  = DovahKitCore::get();
      try {
         auto request = editor.request_form_renumber(*this->stub, this->desiredID);
         request.commit();
      } catch (const exception& ex) {
         this->error = true;
         this->error_text = _explain_error_code(ex.code);
         return;
      }
   }
}