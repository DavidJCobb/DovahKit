#pragma once
#include "./ObjectReference.h"

namespace dovah::loaded_forms {
   class PlacedFlame : public ObjectReference {
      public:
         static constexpr const enum form_type form_type = form_type::flame;
         PlacedFlame(const constructor_params& c) : ObjectReference(form_type, c) {};
   };
}