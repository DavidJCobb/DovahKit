#pragma once
#include <QColor>

namespace dovah {
   class form_stub;
}

namespace ui::types::face_tints {
   struct preset {
      public:
         dovah::face_tint_index_type index = 0;
         float alpha = 1.0;
         struct {
            dovah::form_stub* form = nullptr;
            QColor cached;
         } color;

      public:
         void recache_color();
   };
}
