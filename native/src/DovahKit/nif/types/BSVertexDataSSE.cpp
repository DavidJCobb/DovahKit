#include "BSVertexDataSSE.h"
#include "BSVertexDesc.h"
#include "../reader.h"

// for batch read
#include <type_traits>
#include <intrin.h>
#include "helpers/cpuinfo.h"

namespace nifDK {
   namespace {
      constexpr size_t _bytes_per_vertex(BSVertexDesc::vertex_flags_t flags) {
         using _ = BSVertexDesc::vertex_flag;
         //
         size_t count = 0;
         if ((flags & _::vertex) != 0) {
            count += sizeof(float) * 3; // position
            count += sizeof(float); // bitangent X or unknown
         }
         if ((flags & _::uv) != 0) {
            static_assert(sizeof(Float16) == 2);
            count += sizeof(Float16) * 2; // UV (float16[2])
         }
         if ((flags & _::normals) != 0) {
            count += 3; // normal (byte vector)
            count += 1; // bitangent Y
            if ((flags & _::tangents) != 0) {
               count += 3; // tangent (byte vector)
               count += 1; // bitangent Z
            }
         }
         if ((flags & _::vertex_colors) != 0) {
            count += 4; // RGBA color as bytes
         }
         if ((flags & _::skinned) != 0) {
            count += sizeof(float) * 4; // bone weights
            count += 4; // bone indices
         }
         if ((flags & _::eye_data) != 0) {
            count += sizeof(float); // eye data
         }
         return count;
      }

      inline void _read_byte_vector(file_reader& reader, glm::fvec3& out) {
         std::array<uint8_t, 3> bytes;
         reader.unchecked_read(bytes);
         out.x = bytes[0];
         out.y = bytes[1];
         out.z = bytes[2];
      }
   }

   void BSVertexDataSSE::parse(file_reader& reader, const BSVertexDesc& desc) {
      reader.require_size(_bytes_per_vertex(desc.flags));
      this->unchecked_parse(reader, desc);
   }
   void BSVertexDataSSE::unchecked_parse(file_reader& reader, const BSVertexDesc& desc) {
      using _ = BSVertexDesc::vertex_flag;
      if (desc.has_flag(_::vertex)) {
         reader.unchecked_read(this->vertex);
         if (desc.has_flag(_::tangents)) {
            reader.unchecked_read(this->bitangent.x);
         } else {
            reader.unchecked_read(this->unknown);
         }
      }
      if (desc.has_flag(_::uv)) {
         std::array<Float16, 2> halves;
         reader.unchecked_read(halves);
         this->uv = { (float)halves[0], (float)halves[1] };
      }
      if (desc.has_flag(_::normals)) {
         uint8_t byte;
         //
         _read_byte_vector(reader, this->normal);
         reader.unchecked_read(byte);
         this->bitangent.y = ((float)byte - 128.0) / 127.0;
         if (desc.has_flag(_::tangents)) {
            _read_byte_vector(reader, this->tangent);
            reader.unchecked_read(byte);
            this->bitangent.z = ((float)byte - 128.0) / 127.0;
         }
      }
      if (desc.has_flag(_::vertex_colors)) {
         std::array<uint8_t, 4> components;
         reader.unchecked_read(components[0]);
         reader.unchecked_read(components[1]);
         reader.unchecked_read(components[2]);
         reader.unchecked_read(components[3]);
         this->color.r = (float)components[0] / 255.0F;
         this->color.g = (float)components[1] / 255.0F;
         this->color.b = (float)components[2] / 255.0F;
         this->color.a = (float)components[3] / 255.0F;
      }
      if (desc.has_flag(_::skinned)) {
         reader.unchecked_read(this->bones.weights);
         reader.unchecked_read(this->bones.indices);
      }
      if (desc.has_flag(_::eye_data)) {
         reader.unchecked_read(this->eye_data);
      }
   }

   template<BSVertexDesc::vertex_flags_t flags> void _parse_all(file_reader& reader, const BSVertexDesc& desc, std::vector<BSVertexDataSSE>& list) {
      using _ = BSVertexDesc::vertex_flag;
      //
      constexpr size_t bytes_per_vertex = _bytes_per_vertex(flags);
      reader.require_size(bytes_per_vertex * list.size());
      //
      for (size_t i = 0; i < list.size(); ++i) {
         auto& item = list[i];
         if constexpr ((flags & _::vertex) != 0) {
            reader.unchecked_read(item.vertex);
            if constexpr ((flags & _::tangents) != 0) {
               reader.unchecked_read(item.bitangent.x);
            } else {
               reader.skip(4);
            }
         }
         if constexpr ((flags & _::uv) != 0) {
            std::array<Float16, 2> halves;
            reader.unchecked_read(halves);
            item.uv = { (float)halves[0], (float)halves[1] };
         }
         if constexpr ((flags & _::normals) != 0) {
            uint8_t byte;
            //
            _read_byte_vector(reader, item.normal);
            reader.unchecked_read(byte);
            item.bitangent.y = ((float)byte - 128.0) / 127.0;
            if constexpr ((flags & _::tangents) != 0) {
               _read_byte_vector(reader, item.tangent);
               reader.unchecked_read(byte);
               item.bitangent.z = ((float)byte - 128.0) / 127.0;
            }
         }
         if constexpr ((flags & _::vertex_colors) != 0) {
            std::array<uint8_t, 4> components;
            for (auto& byte : components)
               reader.unchecked_read(byte);
            item.color.r = (float)components[0] / 255.0F;
            item.color.g = (float)components[1] / 255.0F;
            item.color.b = (float)components[2] / 255.0F;
            item.color.a = (float)components[3] / 255.0F;
         }
         if constexpr ((flags & _::skinned) != 0) {
            reader.unchecked_read(item.bones.weights);
            reader.unchecked_read(item.bones.indices);
         }
         if constexpr ((flags & _::eye_data) != 0) {
            reader.unchecked_read(item.eye_data);
         }
      }
   }

   struct _common_flags {
      using flags_t = BSVertexDesc::vertex_flags_t;
      using parse_t = decltype(&_parse_all<0>);
      //
      flags_t flags = 0;
      parse_t func  = nullptr;

      constexpr _common_flags() {}
      constexpr _common_flags(flags_t f, parse_t p) : flags(f), func(p) {}
      template<BSVertexDesc::vertex_flags_t f> static constexpr _common_flags from = ([]() { return _common_flags(f, &_parse_all<f>); })();
   };
   namespace _common_flag_sets {
      using _ = BSVertexDesc::vertex_flag;
      //
      constexpr std::array list = {
         _common_flags::from<_::vertex | _::normals | _::vertex_colors>,                       // common for editor markers
         _common_flags::from<_::vertex | _::normals | _::tangents | _::uv | _::vertex_colors>, // common for ordinary geometry
      };
   }

   /*static*/ void BSVertexDataSSE::parse_all(file_reader& reader, const BSVertexDesc& desc, std::vector<BSVertexDataSSE>& list) {
      using _ = BSVertexDesc::vertex_flag;
      //
      for (auto& set : _common_flag_sets::list) {
         if (desc.flags == set.flags) {
            (set.func)(reader, desc, list);
            return;
         }
      }
      reader.require_size(_bytes_per_vertex(desc.flags) * list.size());
      for (auto& item : list) {
         item.unchecked_parse(reader, desc);
      }
   }
}