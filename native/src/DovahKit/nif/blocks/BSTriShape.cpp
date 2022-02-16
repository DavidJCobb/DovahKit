#include "BSTriShape.h"
#include "BSDismemberSkinInstance.h"
#include "BSShaderProperty.h"
#include "NiAlphaProperty.h"
#include "../notice_code_list.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSTriShape::parse(file_reader& reader) {
      NiAVObject::parse(reader);
      //
      reader.read(this->bounds);
      reader.read_ref(this->skin);
      reader.read_ref(this->properties.shader);
      reader.read_ref(this->properties.alpha);
      reader.read(this->vertex_desc);
      //
      uint32_t tri_count;
      if (reader.user_version<2>() < 130) {
         uint16_t v;
         reader.read(v);
         tri_count = v;
      } else {
         reader.read(tri_count);
      }
      this->triangles.resize(tri_count);
      //
      uint16_t vert_count;
      reader.read(vert_count);
      uint32_t data_size;
      reader.read(data_size);
      this->vertices.resize(vert_count);
      if (reader.user_version<2>() == 130) {
         reader.throw_error(notice_code::unsupported_user_version_2); // TODO: support FO4 vertex data?
      }
      BSVertexDataSSE::parse_all(reader, this->vertex_desc, this->vertices);
      reader.read_vector_contents(this->triangles);
      //
      if (reader.user_version<2>() == 100) {
         uint32_t particle_data_size;
         reader.read(particle_data_size);
         if (particle_data_size > 0) {
            this->particle_data.per_vertex.resize(vert_count);
            this->particle_data.triangles.resize(tri_count);
            reader.read_vector_contents(this->particle_data.per_vertex);
            reader.read_vector_contents(this->particle_data.triangles);
         }
      }
   }
}