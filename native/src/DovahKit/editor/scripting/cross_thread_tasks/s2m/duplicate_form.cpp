#include "duplicate_form.h"
#include "../../../core.h"
#include "../../../../dovah/files/file_load_order.h"
#include "../../../../dovah/notice_code_list.h"
#include "../../../helpers/make_editor_id_for_duplicate.h"

namespace {
   const char* _explain_error_code(dovah::notice_code_t code) {
      using notice_code = dovah::notice_code;
      switch (code) {
         case notice_code::form_is_not_defined_in_active_file:
            return "cannot duplicate the form (or one of its child forms) because it doesn't originate from the active file";
         case notice_code::form_id_unavailable_for_new_form:
            return "cannot duplicate the form (or one of its child forms) because there aren't enough form IDs left in the active file";
         case notice_code::unimplemented_form_type:
            return "cannot duplicate the form (or one of its child forms) because DovahKit doesn't yet support loading forms of this type";
         case notice_code::invalid_parent_child_relationship:
            return "cannot duplicate the form because the specified parent form cannot have a child of this type";
         case notice_code::no_active_file:
            return "cannot duplicate the form because there is neither an active file nor any room in the load order for a new file";
         case notice_code::cannot_sever_references_to_none_stub:
            return "cannot duplicate the form (or one of its child forms) because the form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load";
         case notice_code::exterior_grid_coordinates_already_taken:
            return "cannot duplicate the exterior cell because its parent worldspace already has a cell at the specified coordinates (or 0, 0) if none were specified";
         case notice_code::cannot_create_reference_with_no_parent_cell:
            return "cannot duplicate the reference because no parent cell was supplied (wait, what? shouldn't we have used the original reference's parent?)";
         case notice_code::interior_cell_clone_cannot_have_parent:
            return "cannot duplicate the interior cell if a parent worldspace is specified, as interior cells must have a parent worldspace";
         case notice_code::exterior_cell_clone_must_have_parent:
            return "cannot duplicate the exterior cell because no parent worldspace was supplied (wait, what? shouldn't we have used the original cell's parent?)";
      }
      return "cannot duplicate the form (or one of its child forms) for an unknown reason";
   }
}

namespace editor_script::tasks::s2m {
   /*virtual*/ void duplicate_form::_exec_impl() /*override*/ {
      assert(this->source);
      auto& editor  = DovahKitCore::get();
      auto  request = editor.request_form_duplication();
      request.set_target(this->source);
      request.set_parent_form(this->parent);
      if (this->editorID.empty()) {
         request.editorID = editor_helpers::make_editor_id_for_duplicate(this->source->get_editor_id()).toStdString();
      } else {
         request.editorID = this->editorID;
      }
      request.cell_grid_coordinates.x = this->cell_grid_coordinates.x;
      request.cell_grid_coordinates.y = this->cell_grid_coordinates.y;
      //
      if (request.has_error()) {
         this->error = true;
         //
         auto errors = request.get_error_codes();
         if (errors.size()) {
            this->error_text = _explain_error_code(errors[0]);
         } else {
            this->error_text = _explain_error_code(dovah::default_notice_code);
         }
         return;
      }
      //
      this->result = request.commit();
      //
      if (auto code = request.get_main_form_error_code()) {
         this->error      = true;
         this->error_text = _explain_error_code(code);
         return;
      }
      auto child_errors = request.get_child_form_error_codes();
      if (!child_errors.empty()) {
         this->error      = true;
         this->error_text = _explain_error_code(child_errors[0]);
      }
   }
}