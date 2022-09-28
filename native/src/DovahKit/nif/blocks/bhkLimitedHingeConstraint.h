#pragma once
#include <glm/glm.hpp>
#include "../types/LimitedHingeDescriptor.h"
#include "./bhkConstraint.h"

namespace nifDK::block_types {
   class bhkLimitedHingeConstraint : public bhkConstraint {
      public:
         static constexpr const char* const type_name = "bhkLimitedHingeConstraint";
      public:
         LimitedHingeDescriptor data;

         virtual void parse(file_reader&) override;
   };
}