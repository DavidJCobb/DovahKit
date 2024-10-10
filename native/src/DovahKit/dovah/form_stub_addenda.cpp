#include "form_stub_addenda.h"
#include <algorithm>

namespace dovah {
   void form_stub_addenda::clone_from(const form_stub_addenda& other) {
   }
   void form_stub_addenda::sever_references_to_deleted_form(form_stub& target, bool just_being_flagged) {
      if (this->persistent_cell == &target)
         this->persistent_cell = nullptr;
      if (this->canonical_landscape == &target)
         this->canonical_landscape = nullptr;
      {
         auto& oc     = this->ordered_children;
         auto& list_a = oc.active_file;
         auto& list_d = oc.dependencies;
         list_a.erase(std::remove(list_a.begin(), list_a.end(), &target), list_a.end());
         if (!just_being_flagged) {
            //
            // If the form is being deleted from memory, then we need to remove it from the 
            // master list of ordered children in order to avoid a dangling pointer. However, 
            // if the form is just being *flagged* as deleted, then we need to keep it in 
            // that list, so that when we go to save the active file, we know which other 
            // INFOs to serialize PNAM subrecords for.
            //
            list_d.erase(std::remove(list_d.begin(), list_d.end(), &target), list_d.end());
         }
      }
   }
}