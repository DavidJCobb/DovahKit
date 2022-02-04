#pragma once
#include <array>
#include <glm/glm.hpp>
#include "NiColor.h"

namespace nifDK {
   class file_reader;

   struct BSVertexDesc;

   struct BSVertexDataSSE {
      // Presence of members in serialized data is determined by associated BSVertexDesc
      glm::fvec3 vertex    = { 0, 0, 0 };
      glm::fvec3 normal    = { 0, 0, 0 }; // single-byte floats
      glm::fvec3 tangent   = { 0, 0, 0 }; // single-byte floats
      glm::fvec3 bitangent = { 0, 0, 0 }; // single-byte floats
      NiColorA   color; // "ByteColor4" in nif.xml
      int        unknown   = 0;
      struct {
         glm::fvec4 weights = { 0, 0, 0, 0 }; // hfloat
         std::array<uint8_t, 4> indices = {};
      } bones;
      float eye_data;

      void parse(file_reader&, const BSVertexDesc&);
   };
}