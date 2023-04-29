#include "./key.h"
#include "../windows.h"

namespace cobb::keyboard {
   void key::make_complete() {
      if (!this->has_scan_code()) {
         if (this->has_virtual_keycode()) {
            this->scan_code = MapVirtualKey((UINT)this->vk, MAPVK_VK_TO_VSC_EX);
         }
      } else if (!this->has_virtual_keycode()) {
         this->vk = (virtual_key)MapVirtualKey((UINT)this->scan_code, MAPVK_VSC_TO_VK_EX);
      }
      if (!this->unicode) {
         if (this->has_virtual_keycode()) {
            this->unicode = MapVirtualKeyW((UINT)this->vk, MAPVK_VK_TO_CHAR);
         }
      } else if (this->unicode <= 0xFFFF) {
         if (!this->has_virtual_keycode() && !this->has_scan_code()) {
            *this = from_character(this->unicode, true);
         }
      }
   }

   std::wstring key::get_key_name() const {
      std::wstring out;
      out.resize(128);

      LONG sc = (this->scan_code & 0b1111111) << 16;
      if ((this->scan_code >> 0x18) == 0xE0)
         sc |= 1 << 24; // Set the "extended scan code" flag. Distinguishes right-side modifier keys, the numpad, etc..
      if (this->has_virtual_keycode()) {
         switch (this->vk) {
            case virtual_key::alt:
            case virtual_key::ctrl:
            case virtual_key::shift:
               sc |= 1 << 25; // Set the "I don't care about left or right" bit if our VK has no directionality.
         }
      }

      size_t size = GetKeyNameTextW(sc, out.data(), out.size());
      out.resize(size);
      return out;
   }
   std::string key::get_key_name_ansi() const {
      std::string out;
      out.resize(128);

      LONG sc = (this->scan_code & 0b1111111) << 16;
      if ((this->scan_code >> 0x18) == 0xE0)
         sc |= 1 << 24; // Set the "extended scan code" flag. Distinguishes right-side modifier keys, the numpad, etc..
      if (this->has_virtual_keycode()) {
         switch (this->vk) {
            case virtual_key::alt:
            case virtual_key::ctrl:
            case virtual_key::shift:
               sc |= 1 << 25; // Set the "I don't care about left or right" bit if our VK has no directionality.
         }
      }

      size_t size = GetKeyNameTextA(sc, out.data(), out.size());
      out.resize(size);
      return out;
   }

   /*static*/ key key::from_character(wchar_t ch, bool& requires_alt, bool& requires_ctrl, bool& requires_shift) {
      requires_alt   = false;
      requires_ctrl  = false;
      requires_shift = false;

      auto res   = VkKeyScanW(ch);
      auto flags = res >> 8;
      if (res == 0xFFFF) {
         //
         // This character cannot be entered using a single character key in 
         // combination with one or more common modifier keys.
         //
         return key(ch);
      }
      if (flags & 0x08) {
         //
         // The Hankaku modifier encompasses multiple toggle keys that are not 
         // identifiable/representable using VkKeyScanW's return flags.
         //
         return key(ch);
      }
      requires_shift = flags & 0x01;
      requires_ctrl  = flags & 0x02;
      requires_alt   = flags & 0x04;

      key result;
      result.vk      = (virtual_key)(res & 0xFF);
      result.unicode = ch;
      result.make_complete();
      return result;
   }
   /*static*/ key key::from_character(wchar_t ch, bool allow_modifiers) {
      bool alt;
      bool ctrl;
      bool shift;
      auto res = from_character(ch, alt, ctrl, shift);
      if (!allow_modifiers) {
         if (alt || ctrl)
            return key(ch);
      }
      return res;
   }
}