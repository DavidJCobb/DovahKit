#pragma once
#include <cstdint>
#include <Qt>
#include <QString>

class QKeyEvent;

namespace cobb::qt {
   enum class key_side {
      none,
      left,
      right,
   };

   //
   // This struct identifies a single key, e.g. for use in key combinations. It is not 
   // suitable for typed text; use Qt's APIs for that.
   // 
   // As an example:  on an American keyboard  layout, there is  a key for "=", and to 
   // type  a "+" glyph you  hold Shift and press "=".  However, both symbols  entered 
   // through that "="  key have the same  virtual key code and  scan code on Windows, 
   // yet produce different Qt::Key values, because Qt::Key checks what glyph would be 
   // produced were you to type into a textbox.  This means that you can only identify 
   // the key itself, in isolation, using the OS-level information.
   // 
   // This struct, then, stores native key  information alongside the information that 
   // would be sufficient to identify a typed  character or non-printable key using Qt 
   // alone; and it prioritizes native key information above Qt information. Currently 
   // it only  supports Windows, because that's  the OS I develop on; I don't  own any 
   // machines with  other OSes. If one wanted to add support for other  OSes, though, 
   // this would be a good place to do it.
   //
   struct key {
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

      // we can't default these if we have anonymous struct members
      constexpr bool operator==(const key& other) const {
         if (native.vk != other.native.vk)
            return false;
         if (native.scan != other.native.scan)
            return false;
         if (native.vk == 0) {
            //
            // Only bother testing Qt information if the native information isn't available.
            //
            if (code != other.code)
               return false;
            if (glyph != other.glyph)
               return false;
         }
         return true;
      }

      inline bool empty() const noexcept { return this->glyph.isEmpty() && (this->code == Qt::Key::Key_unknown || this->code == (Qt::Key)0); }
      bool is_modifier_key() const noexcept;

      QString toString(bool localize = true) const;

      void update_native_data();

      static key from_windows_vk(int vk);
      static key from_qt_event(const QKeyEvent*);
   };
}