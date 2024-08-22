#include "./form_list_contains.h"
#include "../form_stub.h"
#include "../forms/FormList.h"

namespace dovah {
   extern bool form_list_contains(dovah::form_stub& formlist, const dovah::form_stub& stub) {
      bool found = false;
      for (auto& pair : formlist.outbound) {
         if (pair.second.other == &stub) {
            found = true;
            break;
         }
      }
      if (!found)
         return false;
      auto loaded = formlist.load().ptr_cast<dovah::loaded_forms::FormList>();
      if (loaded) {
         for (auto& form_use : loaded->contents)
            if (form_use.get_form_stub() == &stub)
               return true;
      }
      return false;
   }
}