#include "./form_creation_request.h"
#include "../files/file_load_order.h"

namespace dovah {
   form_creation_request::form_creation_request(file_load_order& o) : owner(o) {
   }
   form_creation_request::form_creation_request(form_creation_request&& other) : owner(other.owner) {
      this->formID    = other.formID;
      this->form_type = other.form_type;
      this->child_of  = other.child_of;
      this->clone_of  = other.clone_of;
      this->error     = other.error;
      //
      this->editorID  = other.editorID;
      this->cell_grid_coordinates = other.cell_grid_coordinates;
      //
      other.formID = 0;
   }
   form_creation_request::~form_creation_request() {
      this->owner.abandon_form_id_reservation(*this);
   }
   void form_creation_request::set_parent_form(form_stub* parent) {
      this->child_of = parent;
   }
   void form_creation_request::set_parent_form(bare_form_id_t parentID) {
      if (!parentID) {
         this->child_of = nullptr;
         return;
      }
      this->child_of = this->owner.get_form(parentID);
   }
   void form_creation_request::queue_clone(form_stub* original) {
      if (this->clone_of == original)
         return;
      if (original) {
         if (original->form_type != this->form_type)
            return;
      }
      this->clone_of = original;
   }
   form_stub* form_creation_request::commit() {
      return this->owner.commit_form_creation_request(*this);
   }
}