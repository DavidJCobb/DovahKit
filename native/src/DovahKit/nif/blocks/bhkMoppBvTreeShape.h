#pragma once
#include "bhkBvTreeShape.h"

namespace nifDK::block_types {
   class bhkMoppBvTreeShape : public bhkBvTreeShape {
      public:
         static constexpr const char* const type_name = "bhkMoppBvTreeShape";
      public:
         enum class mopp_build_type : uint8_t {
            built_with_chunk_subdivision,    // PS3
            built_without_chunk_subdivision, // PC
            build_type_not_set,
         };

         bhkShape*     subject = nullptr;
         uint32_t      pad04[3];
         float         scale = 1.0;
         struct {
            glm::fvec3      origin;
            float           scale; // relates to quantization
            mopp_build_type build_type = mopp_build_type::build_type_not_set;
            std::vector<uint8_t> data;
         } mopp;

         virtual void parse(file_reader&) override;
   };
}