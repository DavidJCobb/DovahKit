#include "./NiPSysData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiPSysData::parse(file_reader& reader) {
      NiParticlesData::parse(reader);

      bool subclasses_ni_geometry_data = reader.version() != file_version::from_parts<20, 2, 0, 7> || reader.user_version<2>() <= 0;

      auto vertex_count = this->max_particle_count;
      if (reader.user_version<2>() < 34) {
         vertex_count = 0;
      }

      if (subclasses_ni_geometry_data) {
         auto& dst = this->legacy.emplace().particle_descriptions;
         dst.resize(vertex_count);
         for (auto& item : dst) {
            reader.read(item.translation);
            reader.read(item.unknown_01);
            reader.read(item.unknown_02.a);
            reader.read(item.unknown_02.b);
            reader.read(item.unknown_02.c);
            reader.read(item.unknown_03);
         }
      }
      if (reader.version() >= file_version::from_parts<20, 0, 0, 2>) {
         bool presence;
         reader.read(presence);
         if (presence && subclasses_ni_geometry_data) {
            auto& dst = this->legacy.value().rotation_speeds;
            dst.resize(vertex_count);
            reader.read(dst);
         }
      }
      if (subclasses_ni_geometry_data) {
         auto& dst = this->legacy.value();
         reader.read(dst.added_particles_count);
         reader.read(dst.added_particles_base);
      }
   }
}