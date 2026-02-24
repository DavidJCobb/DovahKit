#include "form_stub_addenda.h"
#include <algorithm>
#include "./form_stub_addenda/passkeys/ordered_child_collection.h"

namespace dovah {
   void form_stub_addenda::clone_from(const form_stub_addenda& other) {
   }
   void form_stub_addenda::sever_references_to_deleted_form(form_stub& target, bool just_being_flagged) {
      if (this->persistent_cell == &target)
         this->persistent_cell = nullptr;
      if (this->canonical_landscape == &target)
         this->canonical_landscape = nullptr;
      this->ordered_children._sever_references_to_deleted_form({}, target, just_being_flagged);
   }
}