#include "./preset.h"
#include "dovah/forms/Color.h"
#include "dovah/form_stub.h"

namespace ui::types::face_tints {
   void preset::recache_color() {
      auto* stub = this->color.form;
      if (!stub || stub->form_type != dovah::form_type::color) {
         this->color.cached = QColor(0, 0, 0, 0);
         return;
      }
      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Color>();
      if (!loaded) {
         this->color.cached = QColor(0, 0, 0, 0);
         return;
      }
      this->color.cached = QColor(loaded->color.r, loaded->color.g, loaded->color.b);
   }
}