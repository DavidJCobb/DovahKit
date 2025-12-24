#pragma once
#include "./ObjectReference.h"

namespace dovah::loaded_forms {
   class PlacedCone : public ObjectReference {
      public:
         static constexpr const enum form_type form_type = form_type::cone;
         PlacedCone(const constructor_params& c) : ObjectReference(form_type, c) {};
   };
}