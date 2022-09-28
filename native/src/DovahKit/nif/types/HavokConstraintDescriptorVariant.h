#pragma once
#include <variant>
#include "./hkConstraintType.h"
#include "./BallAndSocketDescriptor.h"
#include "./HingeDescriptor.h"
#include "./LimitedHingeDescriptor.h"
#include "./MalleableDescriptor.h"
#include "./PrismaticDescriptor.h"
#include "./RagdollDescriptor.h"
#include "./StiffSpringDescriptor.h"

namespace nifDK {
   class file_reader;

   struct HavokConstraintDescriptorVariant : public std::variant<
      std::monostate,
      BallAndSocketDescriptor,
      HingeDescriptor,
      LimitedHingeDescriptor,
      MalleableDescriptor,
      PrismaticDescriptor,
      RagdollDescriptor,
      StiffSpringDescriptor
   > {
      void read_data(file_reader&, hkConstraintType);
   };
}