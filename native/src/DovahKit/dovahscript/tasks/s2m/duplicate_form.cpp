#include "./duplicate_form.h"
#include "editor/core.h"
#include "editor/helpers/make_editor_id_for_duplicate.h"
#include "dovah/exceptions/form_creation_failed.h"

namespace {
   using exception  = dovah::exceptions::form_creation_failed;
   using error_code = exception::error_code;

   const char* _explain_error_code(error_code code) {
      switch (code) {
         case error_code::wrong_load_order:
            return "cannot duplicate the form (or one of its child forms) because something impossible happened (wrong_load_order); contact Dovahit's developer";

         case error_code::no_form_id_available:
            return "cannot duplicate the form (or one of its child forms) because there aren't enough form IDs left in the active file";
         case error_code::unimplemented_form_type:
            return "cannot duplicate the form (or one of its child forms) because DovahKit doesn't yet support loading forms of this type";
         case error_code::invalid_parent_child_relationship:
            return "cannot duplicate the form because the specified parent form cannot have a child of this type";
         case error_code::no_active_file:
            return "cannot duplicate the form because there is neither an active file nor any room in the load order for a new file";
         case error_code::exterior_cell_must_have_grid_coordinates:
            return "cannot duplicate the exterior cell because no grid coordinates were specified for the duplicate";
         case error_code::cannot_sever_references_to_none_stub:
            return "cannot duplicate the form (or one of its child forms) because the form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load";
         case error_code::exterior_grid_coordinates_already_taken:
            return "cannot duplicate the exterior cell because its parent worldspace already has a cell at the specified coordinates (or 0, 0) if none were specified";
         case error_code::cannot_create_reference_with_no_parent_cell:
            return "cannot duplicate the reference because no parent cell was supplied (wait, what? shouldn't we have used the original reference's parent?)";
         case error_code::interior_cell_clone_cannot_have_parent:
            return "cannot duplicate the interior cell if a parent worldspace is specified, as interior cells must have a parent worldspace";
         case error_code::exterior_cell_clone_must_have_parent:
            return "cannot duplicate the exterior cell because no parent worldspace was supplied (wait, what? shouldn't we have used the original cell's parent?)";
      }
      return "cannot duplicate the form (or one of its child forms) for an unknown reason";
   }
}

namespace dovahscript::tasks::s2m {
   /*virtual*/ void duplicate_form::_exec_impl() /*override*/ {
      assert(this->source);
      auto request = DovahKitCore::get().request_form_duplication();
      request.set_target(this->source);
      request.set_parent_form(this->parent);
      if (this->editorID.empty()) {
         request.editorID = editor_helpers::make_editor_id_for_duplicate(this->source->get_editor_id()).toStdString();
      } else {
         request.editorID = this->editorID;
      }
      request.cell_grid_coordinates = {
         .x = this->cell_grid_coordinates.x,
         .y = this->cell_grid_coordinates.y,
      };

      try {
         this->result = request.commit();
      } catch (const exception& ex) {
         this->error      = true;
         this->error_text = _explain_error_code(ex.code);
         return;
      }
      if (!this->result) {
         this->error      = true;
         this->error_text = "cannot duplicate the form for an unknown reason";
      }
   }
}