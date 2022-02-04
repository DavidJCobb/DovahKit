#include "BSVertexDataSSE.h"
#include "BSVertexDesc.h"
#include "../reader.h"

namespace nifDK {
   namespace {
      inline void _read_byte_vector(file_reader& reader, glm::fvec3& out) {
         std::array<uint8_t, 3> bytes;
         reader.read(bytes);
         out.x = bytes[0];
         out.y = bytes[1];
         out.z = bytes[2];
      }
   }

   void BSVertexDataSSE::parse(file_reader& reader, const BSVertexDesc& desc) {
      using _ = BSVertexDesc::vertex_flag;
      if (desc.has_flag(_::vertex)) {
         reader.read(this->vertex);
         if (desc.has_flag(_::tangents)) {
            reader.read(this->bitangent.x);
         } else {
            reader.read(this->unknown);
         }
      }
      if (desc.has_flag(_::normals)) {
         uint8_t byte;
         //
         _read_byte_vector(reader, this->normal);
         reader.read(byte);
         this->bitangent.y = byte;
         if (desc.has_flag(_::tangents)) {
            _read_byte_vector(reader, this->tangent);
            reader.read(byte);
            this->bitangent.z = byte;
         }
      }
      if (desc.has_flag(_::vertex_colors)) {
         reader.read(this->color);
      }
      if (desc.has_flag(_::skinned)) {
         reader.read(this->bones.weights);
         reader.read(this->bones.indices);
      }
      if (desc.has_flag(_::eye_data)) {
         reader.read(this->eye_data);
      }
   }
}