#include "./location_is_or_is_inside_of_location.h"
#include "../forms/Location.h"

namespace dovah::utils {
   extern bool location_is_or_is_inside_of_location(form_stub& subject, const form_stub& desired) {
      if (subject.form_type != form_type::location)
         return false;
      if (desired.form_type != form_type::location)
         return false;
      if (&subject == &desired)
         return true;
      
      loaded_form_ptr<loaded_forms::Location> loaded_loc = subject.load().ptr_cast<loaded_forms::Location>();
      while (loaded_loc) {
         auto* parent = loaded_loc->parent_location.get_form_stub();
         if (!parent || parent->form_type != form_type::location)
            break;
         if (parent == &desired)
            return true;
         loaded_loc = parent->load().ptr_cast<loaded_forms::Location>();
      }
      return false;
   }
}