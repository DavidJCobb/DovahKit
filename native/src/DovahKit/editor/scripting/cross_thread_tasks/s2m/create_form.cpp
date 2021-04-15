#include "create_form.h"
#include "../../../core.h"
#include "../../../../dovah/files/file_load_order.h"
#include "../../../../dovah/notice_code_list.h"

namespace {
   const char* _explain_error_code(dovah::notice_code_t code) {
      using notice_code = dovah::notice_code;
      switch (code) {
         case notice_code::unknown_form_type:
            return "cannot create a new form because this is not a valid form type";
         case notice_code::no_active_file:
            return "cannot create a new form because there is neither an active file nor any room in the load order for a new file";
         case notice_code::form_id_unavailable_for_game_setting:
            [[fallthrough]];
         case notice_code::form_id_unavailable_for_new_form:
            return "cannot create a new form because there are no form IDs left in the active file";
         case notice_code::unimplemented_form_type:
            return "cannot create a new form because DovahKit doesn't yet support loading forms of this type";
         case notice_code::invalid_parent_child_relationship:
            return "cannot create a new form because the specified parent form cannot have a child form of this type";
         case notice_code::exterior_grid_coordinates_already_taken:
            return "cannot create a new cell because the specified parent worldspace already has an exterior cell at the desired grid coordinates";
         case notice_code::cannot_create_reference_with_no_parent_cell:
            return "cannot create a new reference unless you specify a parent cell";
         case notice_code::cannot_sever_references_to_none_stub:
            return "cannot create a new form because the form ID that DovahKit wants to use is the target of one or more dangling references, and at least one is outbound from a form that DovahKit doesn't yet know how to load";
      }
      return "cannot create a new form for an unknown reason";
   }
}

namespace editor_script::tasks::s2m {
   /*virtual*/ void create_form::_exec_impl() /*override*/ {
      this->result = nullptr;
      //
      auto& editor  = DovahKitCore::get();
      auto  request = editor.request_form_creation(this->form_type);
      //
      auto code = request.get_error_code();
      if (code != dovah::default_notice_code) {
         this->error      = true;
         this->error_text = _explain_error_code(code);
         return;
      }
      //
      request.set_parent_form(this->parent);
      request.editorID = this->editorID;
      request.cell_grid_coordinates.x = this->cell_grid_coordinates.x;
      request.cell_grid_coordinates.y = this->cell_grid_coordinates.y;
      request.cell_grid_coordinates.present = this->cell_grid_coordinates.present;
      this->result = request.commit();
      //
      code = request.get_error_code();
      if (code != dovah::default_notice_code || !this->result) {
         this->error      = true;
         this->error_text = _explain_error_code(code);
      }
   }
}