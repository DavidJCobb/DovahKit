#pragma once
#include <cstdint>
#include <string>
#ifdef QT_CORE_LIB
   #include <Qt>
#endif
#include "./virtual_key.h"
#ifdef QT_CORE_LIB
   #include "./qt_to_vk.h"
#endif

namespace cobb::keyboard {
   // A data structure capable of identifying a key (when possible) from a virtual keycode, 
   // scan code, or Unicode character code, as well as converting between these.
   //
   // The Win32 API is needed to properly convert between different representations. You 
   // can construct instances of `key` at compile-time, but they will only contain the 
   // representation information you provide. If you're taking compile-time `key`s and 
   // comparing them to run-time `key`s, be sure to run `key::make_complete` on the former.
   struct key {
      virtual_key vk = virtual_key::none;
      uint32_t scan_code = 0;
      uint32_t unicode   = 0;

      constexpr key() {}
      constexpr key(virtual_key v) : vk(v) {
         if (!std::is_constant_evaluated()) {
            this->make_complete();
         }
      }
      explicit constexpr key(char ch) : unicode(ch) {
         if (!std::is_constant_evaluated()) {
            this->make_complete();
         }
      }
      explicit constexpr key(wchar_t ch) : unicode(ch) {
         if (!std::is_constant_evaluated()) {
            this->make_complete();
         }
      }
      #ifdef QT_CORE_LIB
         constexpr key(Qt::Key k) {
            this->vk = qt_to_vk(k);
            if (!std::is_constant_evaluated()) {
               this->make_complete();
            }
         }
      #endif
      key(uint32_t scan) : scan_code(scan) { // intentionally NOT constexpr, as scan codes depend on end-user keyboard layout
         this->make_complete();
      }

      // Strict/exact equality.
      constexpr bool operator==(const key& other) const = default;

      // Loose equality.
      constexpr bool is_same_as(const key& other) const {
         if (this->has_virtual_keycode() && this->vk == other.vk)
            return true;
         if (this->unicode == other.unicode)
            return true;
         if (this->scan_code == other.scan_code)
            return true;
         return false;
      }

      constexpr bool has_scan_code() const { return this->scan_code != 0; }
      constexpr bool has_virtual_keycode() const { return this->vk != virtual_key::none; }
      constexpr bool has_unicode() const { return this->unicode != 0; }

      constexpr bool has_extended_scan_code() const {
         switch (this->scan_code >> 0x18) {
            case 0xE0:
            case 0xE1:
               return true;
         }
         return false;
      }

      constexpr bool empty() const {
         return !(this->has_scan_code() || this->has_virtual_keycode() || this->has_unicode());
      }

      constexpr bool is_alt() const {
         switch (this->vk) {
            case virtual_key::alt:
            case virtual_key::alt_l:
            case virtual_key::alt_r:
               return true;
         }
         return false;
      }
      constexpr bool is_ctrl() const {
         switch (this->vk) {
            case virtual_key::ctrl:
            case virtual_key::ctrl_l:
            case virtual_key::ctrl_r:
               return true;
         }
         return false;
      }
      constexpr bool is_shift() const {
         switch (this->vk) {
            case virtual_key::shift:
            case virtual_key::shift_l:
            case virtual_key::shift_r:
               return true;
         }
         return false;
      }
      constexpr bool is_windows_key() const {
         switch (this->vk) {
            case virtual_key::windows_l:
            case virtual_key::windows_r:
               return true;
         }
         return false;
      }

      constexpr bool is_modifier() const {
         return this->is_alt() || this->is_ctrl() || this->is_shift();
      }

      constexpr bool is_mouse() const {
         switch (this->vk) {
            case virtual_key::mouse_left:
            case virtual_key::mouse_middle:
            case virtual_key::mouse_right:
            case virtual_key::mouse_x1:
            case virtual_key::mouse_x2:
               return true;
         }
         return false;
      }

      void make_complete();

      std::wstring get_key_name() const;     // The scan code must be known.
      std::string get_key_name_ansi() const; // The scan code must be known.

      static key from_character(wchar_t, bool& requires_alt, bool& requires_ctrl, bool& requires_shift);
      static key from_character(wchar_t ch, bool allow_modifiers);
   };
}
