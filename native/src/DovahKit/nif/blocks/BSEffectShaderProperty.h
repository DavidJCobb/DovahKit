#pragma once
#include <array>
#include "BSShaderProperty.h"
#include "../types/NiColor.h"

namespace nifDK::block_types {
   class BSEffectShaderProperty : public BSShaderProperty {
      public:
         static constexpr const char* const type_name = "BSEffectShaderProperty";
      public:
         std::array<uint32_t, 2> shader_flags = {};
         struct {
            glm::fvec2 uv_offset;
            glm::fvec2 uv_scale;
            uint8_t    clamp_mode = 3;
            std::string path;
            std::string greyscale;
            std::string environment_map;
            std::string normal;
            std::string environment_mask;
            float env_map_scale;
         } texture;
         uint8_t lighting_influence;
         uint8_t env_map_min_lod;
         uint8_t unknown;
         struct {
            struct {
               float start = 1.0;
               float stop  = 1.0;
            } angle;
            struct {
               float start = 1.0;
               float stop  = 0.0;
            } opacity;
         } falloff;
         struct {
            NiColorA color;
            float rgb_mult;
         } emissive;
         float soft_falloff_depth;

         virtual void parse(file_reader&) override;
   };
}