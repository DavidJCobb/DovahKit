#include "HavokConstraintDescriptorVariant.h"
#include "../reader.h"

namespace nifDK {
   void HavokConstraintDescriptorVariant::read_data(file_reader& reader, hkConstraintType type) {
      switch (type) {
         case hkConstraintType::ball_and_socket:
            reader.read(this->emplace<BallAndSocketDescriptor>());
            break;
         case hkConstraintType::hinge:
            reader.read(this->emplace<HingeDescriptor>());
            break;
         case hkConstraintType::limited_hinge:
            reader.read(this->emplace<LimitedHingeDescriptor>());
            break;
         case hkConstraintType::malleable:
            reader.read(this->emplace<MalleableDescriptor>());
            break;
         case hkConstraintType::prismatic:
            reader.read(this->emplace<PrismaticDescriptor>());
            break;
         case hkConstraintType::ragdoll:
            reader.read(this->emplace<RagdollDescriptor>());
            break;
         case hkConstraintType::stiff_spring:
            reader.read(this->emplace<StiffSpringDescriptor>());
            break;
      }
   }
}