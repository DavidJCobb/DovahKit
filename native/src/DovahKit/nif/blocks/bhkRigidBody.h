#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "./bhkEntity.h"
#include "../types/HavokMaterial.h"
#include "../types/hkDeactivatorType.h"
#include "../types/hkMotionType.h"
#include "../types/hkQualityType.h"
#include "../types/hkResponseType.h"
#include "../types/hkSolverDeactivation.h"

namespace nifDK::block_types {
   class bhkRigidBody : public bhkEntity {
      public:
         static constexpr const char* const type_name = "bhkRigidBody";
      public:
         struct collision_response {
            hkResponseType type = hkResponseType::simple_contact;
            uint8_t        pad01;
            uint16_t       process_contact_callback_delay = 0xFFFF;

            void read(file_reader&);
         };

         collision_response response;
         uint32_t       unk_int_1;
         HavokFilter    filter;
         uint8_t        pad0C[4];
         uint32_t       unk_int_2;
         collision_response response_alt;
         uint32_t       unk_int_3;
         glm::fvec4     translation;
         glm::fquat     rotation;
         struct {
            glm::fvec4 linear;
            glm::fvec4 angular;
         } velocity;
         glm::fmat3x4 inertia_tensor; // yes, 3x4; Havok uses tons of vec4s for SIMD shenanigans
         glm::fvec4 center_of_mass;
         float mass = 1.0F; // kilograms; 0 == immovable
         struct {
            float linear  = 0.1F; // velocity reduction (inverse multiplier, i.e. 0.1 = -10%) per second
            float angular = 0.05F;
         } damping;
         float time_factor = 1.0F;
         float gravity_factor = 1.0F;
         float friction = 0.5F;
         float rolling_friction_mult;
         float restitution = 0.4F;
         struct {
            float linear  = 104.4F;
            float angular =  31.57F;
         } max_velocity;
         float penetration_depth = 0.15F;
         hkMotionType motion_type = hkMotionType::dynamic;
         struct {
            hkDeactivatorType type = hkDeactivatorType::never;
            bool enabled = true;
            hkSolverDeactivation solver = hkSolverDeactivation::off;
         } deactivation;
         hkQualityType quality = hkQualityType::fixed;
         float unknown_float;
         uint8_t unknown_bytes_x12[12];
         uint8_t unknown_bytes_x4[4];
         std::vector<bhkSerializable*> constraints;
         uint32_t body_flags = 0;

         virtual void parse(file_reader&) override;
   };
}