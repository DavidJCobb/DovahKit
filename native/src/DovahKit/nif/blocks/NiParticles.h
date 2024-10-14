#pragma once
#include <optional>
#include "NiGeometry.h"
#include "../types/BSVertexDesc.h"

namespace nifDK::block_types {
   class NiParticles : public NiGeometry {
      public:
         static constexpr const char* const type_name = "NiParticles";
      public:
         std::optional<BSVertexDesc> vertex_description; // SSE

         virtual void parse(file_reader&) override;
   };
}