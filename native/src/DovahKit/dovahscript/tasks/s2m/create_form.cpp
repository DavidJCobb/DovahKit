#include "./create_form.h"
#include "editor/core.h"
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/utils/form_type_is_cell_child.h"

// for forms whose parentage is unconventional
#include "dovah/form_stubs/helpers/get_unique_outbound_use.h"
#include "dovah/forms/DialogueBranch.h"
#include "dovah/forms/Scene.h"
#include "dovah/forms/Topic.h"
#include "dovah/use_info/entry_flags/dialogue_branch.h"

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
      
      if (!this->check_non_group_parenthood()) {
         this->error = true;
         if (this->parent) {
            this->error_text = _explain_error_code(error_code::invalid_parent_child_relationship);
         } else {
            if (dovah::form_type_is_cell_child(this->form_type)) {
               this->error_text = _explain_error_code(error_code::cannot_create_reference_with_no_parent_cell);
            } else {
               switch (this->form_type) {
                  case dovah::form_type::dialogue_branch:
                  case dovah::form_type::scene:
                     this->error_text = "cannot create a form of this type unless you specify a parent quest";
                     break;
                  case dovah::form_type::topic:
                     this->error_text = "cannot create a form of this type unless you specify a parent quest or parent dialogue branch";
                     break;
                  default:
                     this->error_text = "cannot create a form of this type unless you specify an appropriate parent form";
                     break;
               }
            }
         }
         return;
      }

      auto& editor  = DovahKitCore::get();
      try {
         auto request = editor.request_form_creation(this->form_type);
         if (this->parenthood_is_via_record_groups()) {
            request.set_parent_form(this->parent);
         }
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
      if (this->result) {
         if (this->parent && !this->parenthood_is_via_record_groups()) {
            this->set_up_non_group_parenthood();
         }
      } else {
         this->error      = true;
         this->error_text = "cannot create a new form for an unknown reason";
      }
   }

   bool create_form::parenthood_is_via_record_groups() const {
      if (dovah::form_type_is_cell_child(this->form_type))
         return true;
      switch (this->form_type) {
         case dovah::form_type::cell:
         case dovah::form_type::topic_info:
            return true;
      }
      return false;
   }
   bool create_form::check_non_group_parenthood() const {
      switch (this->form_type) {
         case dovah::form_type::dialogue_branch:
            if (!this->parent)
               return false;
            if (this->parent->form_type != dovah::form_type::quest)
               return false;
            break;
         case dovah::form_type::scene:
            if (!this->parent)
               return false;
            if (this->parent->form_type != dovah::form_type::quest)
               return false;
            break;
         case dovah::form_type::topic:
            if (!this->parent)
               return false;
            if (this->parent->form_type != dovah::form_type::quest)
               if (this->parent->form_type != dovah::form_type::dialogue_branch)
                  return false;
            break;
      }
      return true;
   }
   void create_form::set_up_non_group_parenthood() {
      if (!this->parent)
         return;
      switch (this->form_type) {
         case dovah::form_type::dialogue_branch:
            {
               auto loaded = this->result->load().ptr_cast<dovah::loaded_forms::DialogueBranch>();
               if (loaded) {
                  loaded->owning_quest.set(*loaded, this->parent);
               }
            }
            break;
         case dovah::form_type::scene:
            {
               auto loaded = this->result->load().ptr_cast<dovah::loaded_forms::Scene>();
               if (loaded) {
                  loaded->owning_quest.set(*loaded, this->parent);
               }
            }
            break;
         case dovah::form_type::topic:
            {
               auto loaded = this->result->load().ptr_cast<dovah::loaded_forms::Topic>();
               if (loaded) {
                  if (this->parent->form_type == dovah::form_type::quest) {
                     loaded->owning_forms.quest.set(*loaded, this->parent);
                  } else if (this->parent->form_type == dovah::form_type::dialogue_branch) {
                     loaded->owning_forms.branch.set(*loaded, this->parent);

                     auto* quest = dovah::form_stub_helpers::get_unique_outbound_use<dovah::use_info::entry_flags::dialogue_branch::parent_quest>(*this->parent);
                     loaded->owning_forms.quest.set(*loaded, quest);
                  }
               }
            }
            break;
      }
   }
}