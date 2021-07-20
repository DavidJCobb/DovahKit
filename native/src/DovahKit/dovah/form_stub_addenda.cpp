#include "form_stub_addenda.h"
#include <algorithm>

namespace dovah {
   void form_stub_addenda::clone_from(const form_stub_addenda& other) {
   }
   void form_stub_addenda::sever_references_to(form_stub& target) {
      if (this->persistent_cell == &target)
         this->persistent_cell = nullptr;
      if (this->canonical_landscape == &target)
         this->canonical_landscape = nullptr;
      auto& oc = this->ordered_children;
      oc.erase(std::remove(oc.begin(), oc.end(), &target), oc.end());
   }
}