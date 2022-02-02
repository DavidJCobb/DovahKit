#pragma once
#include <array>
#include "NiShadeProperty.h"
#include "../types/NiColor.h"

namespace nifDK::block_types {
   class BSShaderProperty : public NiShadeProperty {
      public:
         static constexpr const char* const type_name = "BSShaderProperty";
      public:
         enum shader_type {
            tall_grass  = 0,
            standard    = 1,
            sky         = 10,
            skin        = 14,
            water       = 17,
            lighting_3  = 29, // Lighting 3.0
            tiled       = 32,
            no_lighting = 33,
         };

         struct {
            shader_type type = shader_type::standard;
            std::array<uint32_t, 2> shader_flags = {};
            float env_map_scale = 1.0;
         } legacy_data; // user version 2 <= 34

         virtual void parse(file_reader&) override;
   };
}