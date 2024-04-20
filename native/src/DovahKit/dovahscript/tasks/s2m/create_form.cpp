#include "./create_form.h"
#include "editor/core.h"
#include "dovah/exceptions/form_creation_failed.h"

namespace {
   using exception  = dovah::exceptions::form_creation_failed;
   using error_code = exception::error_code;

   const char* _explain_error_code(error_code code) {
      switch (code) {
         case error_code::wrong_load_order:
            return "cannot create a new form because something impossible happened (wrong_load_order); contact Dovahit's developer";

         case error_code::invalid_form_type:
            return "cannot create a new form because this is not a valid form type";
         case error_code::no_active_file:
            return "cannot create a new form because there is neither an active file nor any room in the load order for a new file";
         case error_code::no_form_id_available:
            return "cannot create a new form because there are no form IDs left in the active file";
         case error_code::unimplemented_form_type:
            return "cannot create a new form because DovahKit doesn't yet support loading forms of this type";
         case error_code::exterior_cell_must_have_grid_coordinates:
            return "cannot create a new exterior cell because no grid coordinates were specified";
         case error_code::invalid_parent_child_relationship:
            return "cannot create a new form because the specified parent form cannot have a child form of this type";
         case error_code::exterior_grid_coordinates_already_taken:
            return "cannot create a new cell because the specified parent worldspace already has an exterior cell at the desired grid coordinates";
         case error_code::cannot_create_reference_with_no_parent_cell:
            return "cannot create a new reference unless you specify a parent cell";
         case error_code::cannot_sever_references_to_none_stub:
            return "cannot create a new form because the form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load";
      }
      return "cannot create a new form for an unknown reason";
   }
}

namespace dovahscript::tasks::s2m {
   /*virtual*/ void create_form::_exec_impl() /*override*/ {
      this->result = nullptr;
      //
      auto& editor  = DovahKitCore::get();
      try {
         auto request = editor.request_form_creation(this->form_type);
         request.set_parent_form(this->parent);
         request.editorID = this->editorID;
         request.cell_grid_coordinates = {
            .x = this->cell_grid_coordinates.x,
            .y = this->cell_grid_coordinates.y,
         };
         this->result = request.commit();
      } catch (const exception& ex) {
         this->error      = true;
         this->error_text = _explain_error_code(ex.code);
         return;
      }
      if (!this->result) {
         this->error      = true;
         this->error_text = "cannot create a new form for an unknown reason";
      }
   }
}