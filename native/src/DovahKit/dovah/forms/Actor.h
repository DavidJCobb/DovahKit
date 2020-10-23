#pragma once
#include "ObjectReference.h"

namespace dovah::loaded_forms {
   class Actor : public ObjectReference {
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr form_type_t form_type = form_type::actor;
         Actor() : ObjectReference(form_type) {};
   };
}