#pragma once
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include "NiColor.h"

namespace nifDK {
   class file_reader;

   struct BSVertexDesc;

   struct BSVertexDataSSE {
      struct byte_color {
         uint8_t r;
         uint8_t g;
         uint8_t b;
         uint8_t a;
      };

      // Presence of members in serialized data is determined by associated BSVertexDesc
      glm::fvec3 vertex    = { 0, 0, 0 };
      glm::fvec2 uv        = { 0, 0 };
      glm::fvec3 normal    = { 0, 0, 0 }; // single-byte floats
      glm::fvec3 tangent   = { 0, 0, 0 }; // single-byte floats
      glm::fvec3 bitangent = { 0, 0, 0 }; // single-byte y/z
      NiColorA   color     = { 255, 255, 255, 255 };
      int        unknown   = 0;
      struct {
         glm::fvec4 weights = { 0, 0, 0, 0 }; // hfloat
         std::array<uint8_t, 4> indices = {};
      } bones;
      float eye_data;

      void parse(file_reader&, const BSVertexDesc&);
      void unchecked_parse(file_reader&, const BSVertexDesc&);

      static void parse_all(file_reader&, const BSVertexDesc&, std::vector<BSVertexDataSSE>&);
   };
}