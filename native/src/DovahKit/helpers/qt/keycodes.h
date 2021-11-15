#pragma once
#include <Qt>
#include <QString>

namespace cobb::qt {
   enum class key_side {
      none,
      left,
      right,
   };
   struct key { // single key e.g. for key combinations; not suitable for typed text
      key() {}
      key(Qt::Key, key_side s = key_side::none);
      key(QChar);

      Qt::Key  code  = Qt::Key::Key_unknown;
      key_side side  = key_side::none;
      QString  glyph;
      struct {
         uint32_t scan = 0;
         uint32_t vk   = 0;
      } native;

      inline bool empty() const noexcept { return this->glyph.isEmpty() && (this->code == Qt::Key::Key_unknown || this->code == (Qt::Key)0); }

      void update_native_data();

      static key from_windows_vk(int vk);
   };
}