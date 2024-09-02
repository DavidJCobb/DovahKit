#pragma once
#include <string>
#include <vector>
#include "dovah/data/face_tints.h"
#include "./preset.h"

namespace dovah {
   class form_stub;
}

namespace ui::types::face_tints {
   struct layer {
      public:
         dovah::face_tint_index_type index = 0;
         dovah::face_tint_type       type  = dovah::face_tint_type::none;
         std::string                 texture;

         dovah::form_stub* default_color = nullptr;
         std::vector<preset> presets;

      public:
         void recache_all_colors();
         void recache_color(dovah::form_stub&);

         constexpr const preset* preset_by_index(dovah::face_tint_index_type) const;
         constexpr preset* preset_by_index(dovah::face_tint_index_type);
   };
}

#include "./layer.inl"