#pragma once
#include <array>
#include <optional>
#include <vector>
#include "NiParticlesData.h"

namespace nifDK::block_types {
   class NiPSysData : public NiParticlesData {
      public:
         static constexpr const char* const type_name = "NiPSysData";
      public:
         struct particle_description { // ParticleDesc
            glm::vec3 translation;
            std::array<float, 3> unknown_01 = {};
            struct {
               float a = 0.9F;
               float b = 0.9F;
               float c = 3.0F;
            } unknown_02;
            int32_t unknown_03 = 0;
         };

         struct legacy_data {
            std::vector<particle_description> particle_descriptions;
            std::vector<float> rotation_speeds;
            uint16_t added_particles_count = 0;
            uint16_t added_particles_base  = 0;
         };

      public:
         std::optional<legacy_data> legacy;

         virtual void parse(file_reader&) override;
   };
}