#pragma once
#include <vector>
#include "./computed_keyframe.h"
#include "./keyframe.h"
namespace dovah::loaded_forms {
   class ImagespaceModifier;
}

namespace ui::types::imagespace_modifier {
   class keyframe_collection {
      public:
         float duration = 1.0F;
         std::vector<keyframe> keyframes;

      public:
         // A keyframe "position" is a value in the range [0, 1]. To get the 
         // keyframe's timestamp, multiply that value by the total duration.
         keyframe& get_or_create_keyframe(float position);

         // If `is_timestamp` is true, then `at` is a timestamp. Otherwise, 
         // `at` is a position.
         computed_keyframe get_computed_keyframe(float at, bool is_timestamp) const noexcept;

         void import_data(const dovah::loaded_forms::ImagespaceModifier&);
         void export_data(dovah::loaded_forms::ImagespaceModifier&) const;

         void strip_empty_keyframes();
   };
}