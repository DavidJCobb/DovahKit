#include "./layer.h"
#include "dovah/forms/Color.h"
#include "dovah/form_stub.h"

namespace ui::types::face_tints {
   void layer::recache_all_colors() {
      for (auto& item : this->presets)
         item.recache_color();
   }
   void layer::recache_color(dovah::form_stub& stub) {
      if (stub.form_type != dovah::form_type::color)
         return;
      QColor cached;
      {
         auto loaded = stub.load().ptr_cast<dovah::loaded_forms::Color>();
         if (!loaded)
            return;
         cached = QColor(loaded->color.r, loaded->color.g, loaded->color.b);
      }
      for (auto& item : this->presets)
         if (item.color.form == &stub)
            item.color.cached = cached;
   }
}