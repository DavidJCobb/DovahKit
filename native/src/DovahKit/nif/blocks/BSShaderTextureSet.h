#pragma once
#include <array>
#include <string>
#include "NiObject.h"

namespace nifDK::block_types {
   class BSShaderTextureSet : public NiObject {
      public:
         static constexpr const char* const type_name = "BSShaderTextureSet";
      public:
         static constexpr size_t supported_texture_count = 8;

         union texture_list {
            std::array<std::string, supported_texture_count> list = {};
            struct {
               std::string diffuse;
               std::string normal;
               std::string glow;
               std::string height;
               std::string cubemap;
               std::string environment_mask;
               std::string subsurface;
               std::string backlighting;
            };

            ~texture_list() {
               for (auto& s : this->list)
                  s.~basic_string();
            }
         };
         
         texture_list textures;

         virtual void parse(file_reader&) override;
   };
}