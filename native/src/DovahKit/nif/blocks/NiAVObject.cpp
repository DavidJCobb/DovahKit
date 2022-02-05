#include "NiAVObject.h"
#include "../reader.h"

#include "NiCollisionObject.h"

namespace nifDK::block_types {
   void NiAVObject::parse(file_reader& reader) {
      NiObjectNET::parse(reader);
      reader.read(this->flags);
      reader.read(this->transform);
      if (reader.version() <= file_version::from_parts<4, 2, 2, 0>) {
         reader.skip(sizeof(float) * 3); // velocity
      }
      if (reader.user_version<2>() <= 34) {
         uint32_t count;
         reader.read(count);
         reader.skip(4 * count); // skip NiProperty refs for now; this NIF version is too old for us to care about
      }
      if (reader.version() <= file_version::from_parts<2, 3, 0, 0>) {
         reader.skip(sizeof(uint32_t) * 4 + 1); // unknown uint32_t[4] and unknown byte
      }
      if (reader.version() <= file_version::from_parts<4, 2, 2, 0>) {
         bool presence;
         reader.read(presence);
         if (presence) {
            struct _bounding_volume {
               enum class shape : int32_t {
                  undefined = -1,
                  sphere    =  0,
                  box       =  1, // OBB, not AABB
                  capsule   =  2,
                  union_vol =  4,
                  halfspace =  5,
               };
               //
               uint32_t type;
               //
               void read(file_reader& reader) {
                  reader.read(this->type);
                  switch ((shape)this->type) {
                     case shape::sphere:
                        reader.skip(sizeof(float) * 4); // NiBound
                        break;
                     case shape::box:
                        reader.skip(sizeof(float) * 3); // glm::fvec3 center
                        reader.skip((sizeof(float) * 3) * 3); // glm::fvec3[3] axes
                        reader.skip(sizeof(float) * 3); // glm::fvec3 extent
                        break;
                     case shape::capsule:
                        reader.skip(sizeof(float) * 3); // glm::fvec3 center
                        reader.skip(sizeof(float) * 3); // glm::fvec3 origin
                        reader.skip(sizeof(float)); // extent
                        reader.skip(sizeof(float)); // radius
                        break;
                     case shape::union_vol:
                        {
                           uint32_t count;
                           reader.read(count);
                           for (uint32_t i = 0; i < count; ++i)
                              _bounding_volume().read(reader);
                        }
                        break;
                     case shape::halfspace:
                        reader.skip(sizeof(float) * 3); // glm::fvec3 NiPlane::normal
                        reader.skip(sizeof(float));     // float      NiPlane::constant
                        reader.skip(sizeof(float) * 3); // glm::fvec3 center
                        break;
                  }
               }
            };
            //
            _bounding_volume().read(reader);
         }
      }
      if (reader.version() >= file_version::from_parts<10, 0, 1, 0>) {
         reader.read_ref(this->collision);
      }
   }
}