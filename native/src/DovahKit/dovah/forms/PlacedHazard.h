#pragma once
#include "./ObjectReference.h"

namespace dovah::loaded_forms {
   class PlacedHazard : public ObjectReference {
      public:
         static constexpr const enum form_type form_type = form_type::placed_hazard;
         PlacedHazard(const constructor_params& c) : ObjectReference(form_type, c) {};
   };
}