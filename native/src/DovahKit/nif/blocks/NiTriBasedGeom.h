#pragma once
#include "_DKVulkanInterface.h"
#include "NiGeometry.h"

namespace nifDK::block_types {
   class NiTriBasedGeom : public NiGeometry, public virtual block_interfaces::_DKVulkanMeshInterface {
      public:
         static constexpr const char* const type_name = "NiTriBasedGeom";
      public:
         //
         // The "Num Triangles" field is part of this class, but I'd prefer to have a vector 
         // of triangles... and the actual loaded triangle list is on subclasses. Therefore 
         // each subclass will have to load the triangle count manually. C'est la vie.
         //
   };
}