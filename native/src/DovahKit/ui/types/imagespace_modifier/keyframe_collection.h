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
         keyframe& get_or_create_keyframe(float timestamp);

         computed_keyframe get_computed_keyframe(float timestamp) const noexcept;

         void import_data(const dovah::loaded_forms::ImagespaceModifier&);
         void export_data(dovah::loaded_forms::ImagespaceModifier&) const;

         void strip_empty_keyframes();
   };
}