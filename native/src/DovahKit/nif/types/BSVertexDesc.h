#pragma once
#include <array>
#include <cstdint>
#include "helpers/enum_flags.h"

namespace nifDK {
   class file_reader;

   struct BSVertexDesc {
      struct vertex_flag {
         enum type : uint16_t {
            vertex         = (1 <<  4),
            uv             = (1 <<  5),
            uv_2           = (1 <<  6),
            normals        = (1 <<  7),
            tangents       = (1 <<  8),
            vertex_colors  = (1 <<  9),
            skinned        = (1 << 10),
            land_data      = (1 << 11),
            eye_data       = (1 << 12),
            instance       = (1 << 13),
            full_precision = (1 << 14),
         };
      };
      using vertex_flags_t = std::underlying_type_t<vertex_flag::type>;
      
      std::array<uint8_t, 5> unk00;
      vertex_flags_t flags;
      uint8_t unk07;

      inline bool has_flag(vertex_flag::type f) const { return (this->flags & (vertex_flags_t)f) != 0; }

      void parse(file_reader&);
   };
}