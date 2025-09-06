#pragma once
#include <vector>
#include "NiExtraData.h"
#include "dovah/data/furniture/animation_type.h"
#include "dovah/data/furniture/entry_point.h"

namespace nifDK::block_types {
   class BSFurnitureMarkerNode : public NiExtraData {
      public:
         static constexpr const char* const type_name = "BSFurnitureMarkerNode";

         using animation_type       = dovah::furniture::animation_type;
         using animation_type_flags = cobb::enum_flags<animation_type, 16>;
         using entry_point          = dovah::furniture::entry_point;
         using entry_point_flags    = cobb::enum_flags<entry_point, 16>;

         struct marker {
            glm::fvec3           position = { 0, 0, 0 };
            float                yaw = 0;
            animation_type_flags animation_types;
            entry_point_flags    entry_points;
         };

      public:
         std::vector<marker> markers;

         virtual void parse(file_reader&) override;
   };
}