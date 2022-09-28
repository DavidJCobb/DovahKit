#pragma once
#include "bhkConvexShape.h"
#include "../types/hkWorldObjCinfoProperty.h"

namespace nifDK::block_types {
   class bhkConvexVerticesShape : public bhkConvexShape {
      public:
         static constexpr const char* const type_name = "bhkConvexVerticesShape";
      public:
         hkWorldObjCinfoProperty vertex_property;
         hkWorldObjCinfoProperty normal_property;
         std::vector<glm::fvec4> vertices; // lexicographically sorted. w-component is unused (always zero)
         std::vector<glm::fvec4> normals;  // see comment below

         //
         // The normals are half-spaces as determined by the vertices. The first three 
         // vector components define the normal pointing to the exterior. The fourth 
         // component is the signed distance of the separating plane to the origin: 
         // given any vertex on the separating plane, `v`, and the normal, `n`, it's 
         // equal to the negation of the dot product between `v` and `n`.
         // 
         // These, too, are lexicographically sorted.
         //

         virtual void parse(file_reader&) override;
   };
}