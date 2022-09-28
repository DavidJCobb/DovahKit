#pragma once
#include "bhkRigidBody.h"

namespace nifDK::block_types {
   class bhkRigidBodyT : public bhkRigidBody {
      public:
         static constexpr const char* const type_name = "bhkRigidBodyT";
   };
}