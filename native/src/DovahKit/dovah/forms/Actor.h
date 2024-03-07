#pragma once
#include "ObjectReference.h"

namespace dovah::loaded_forms {
   class Actor : public ObjectReference {
      //
      // Intentionally minimal for now.
      //
      public:
         static constexpr const enum form_type form_type = form_type::actor;
         Actor(const constructor_params& c) : ObjectReference(form_type, c) {};
   };
}