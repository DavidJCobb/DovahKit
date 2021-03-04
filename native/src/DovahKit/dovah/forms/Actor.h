#pragma once
#include "ObjectReference.h"

namespace dovah::loaded_forms {
   class Actor : public ObjectReference {
      #include "impl/form_subclass_components.txt"
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::actor;
         Actor(const constructor_params& c) : ObjectReference(form_type, c) {};
   };
}