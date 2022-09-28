#pragma once
#include "bhkEntity.h"

namespace nifDK::block_types {
   class bhkConstraint : public bhkEntity {
      public:
         static constexpr const char* const type_name = "bhkConstraint";
      public:
         std::vector<bhkEntity*> subjects;
         uint32_t priority = 1;

         virtual void parse(file_reader&) override;
   };
}