#include "keycodes.h"
#include <array>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include "../windows.h"

namespace {
   namespace mappings {
      constexpr auto vk_to_qt = ([]() {
         constexpr auto unknown  = Qt::Key::Key_unknown;
         constexpr auto is_glyph = (Qt::Key)0;
         //
         std::array<Qt::Key, VK_OEM_CLEAR + 1> list = {};
         for (auto& v : list)
            v = unknown;
         //
         // 01 is mouse left   // VK_LBUTTON 
         // 02 is mouse right  // VK_RBUTTON
         list[VK_CANCEL]              = Qt::Key::Key_Cancel;             // 03
         // 04 is mouse middle // VK_MBUTTON
         // 05 is mouse X1     // VK_XBUTTON1
         // 06 is mouse X2     // VK_XBUTTON2
         list[VK_BACK]                = Qt::Key::Key_Backspace;          // 08
         list[VK_TAB]                 = Qt::Key::Key_Tab;                // 09
         // 0A is undefined
         // 0B is undefined
         list[VK_CLEAR]               = Qt::Key::Key_Clear;              // 0C
         list[VK_RETURN]              = Qt::Key::Key_Return;             // 0D
         // 0E is undefined
         // 0F is undefined
         list[VK_SHIFT]               = Qt::Key::Key_Shift;              // 10
         list[VK_CONTROL]             = Qt::Key::Key_Control;            // 11
         list[VK_MENU]                = Qt::Key::Key_Alt;                // 12
         list[VK_PAUSE]               = Qt::Key::Key_Pause;              // 13
         list[VK_CAPITAL]             = Qt::Key::Key_CapsLock;           // 14
         // 15 is VK_KANA == VK_HANGUL
         // 16 is VK_IME_ON
         // 17 is VK_JUNJA
         // 18 is VK_FINAL
         // 19 is VK_HANJA == VK_KANJI
         // 1A is VK_IME_OFF
         list[VK_ESCAPE]              = Qt::Key::Key_Escape;             // 1B
         // 1C is VK_CONVERT
         // 1D is VK_NONCONVERT
         // 1E is VK_ACCEPT
         list[VK_MODECHANGE]          = Qt::Key::Key_Mode_switch;        // 1F
         list[VK_SPACE]               = Qt::Key::Key_Space;              // 20
         list[VK_PRIOR]               = Qt::Key::Key_PageUp;             // 21
         list[VK_NEXT]                = Qt::Key::Key_PageDown;           // 22
         list[VK_END]                 = Qt::Key::Key_End;                // 23
         list[VK_HOME]                = Qt::Key::Key_Home;               // 24
         list[VK_LEFT]                = Qt::Key::Key_Left;               // 25
         list[VK_UP]                  = Qt::Key::Key_Up;                 // 26
         list[VK_RIGHT]               = Qt::Key::Key_Right;              // 27
         list[VK_DOWN]                = Qt::Key::Key_Down;               // 28
         list[VK_SELECT]              = Qt::Key::Key_Select;             // 29
         list[VK_PRINT]               = Qt::Key::Key_Pause;              // 2A
         list[VK_EXECUTE]             = Qt::Key::Key_Execute;            // 2B
         list[VK_SNAPSHOT]            = Qt::Key::Key_Print;              // 2C // print screen
         list[VK_INSERT]              = Qt::Key::Key_Insert;             // 2D
         list[VK_DELETE]              = Qt::Key::Key_Delete;             // 2E
         list[VK_HELP]                = Qt::Key::Key_Help;               // 2F
         for (size_t i = 0; i < 10; ++i) {
            //list[0x30 + i] = (Qt::Key)(Qt::Key::Key_0 + i);
            list[0x30 + i] = is_glyph;
         }
         // 3A - 40 are undefined
         for (size_t i = 0; i < 26; ++i) {
            //list[0x41 + i] = (Qt::Key)(Qt::Key::Key_A + i);
            list[0x41 + i] = is_glyph;
         }
         list[VK_LWIN]                = Qt::Key::Key_Meta;               // 5B // Win left
         list[VK_RWIN]                = Qt::Key::Key_Meta;               // 5C // Win right
         list[VK_APPS]                = Qt::Key::Key_Menu;               // 5D
         // 5E is reserved
         list[VK_SLEEP]               = Qt::Key::Key_Sleep;              // 5F
         for (size_t i = 0; i < 10; ++i) {
            list[VK_NUMPAD0 + i] = (Qt::Key)(Qt::Key::Key_0 + i);
         }
         list[VK_MULTIPLY]            = Qt::Key::Key_Asterisk;           // 6A
         list[VK_ADD]                 = Qt::Key::Key_Plus;               // 6B
         // 6C is VK_SEPARATOR (Qt source says locale-dependent)
         list[VK_SUBTRACT]            = Qt::Key::Key_Minus;              // 6D
         // 6E is VK_DECIMAL (Qt source says locale-dependent)
         list[VK_DIVIDE]              = Qt::Key::Key_Slash;              // 6F
         for (size_t i = 0; i < 24; ++i) { // 70 - 87
            list[VK_F1 + i] = (Qt::Key)(Qt::Key::Key_F1 + i);
         }
         // 88 - 8F are undefined
         list[VK_NUMLOCK]             = Qt::Key::Key_NumLock;            // 90
         list[VK_SCROLL]              = Qt::Key::Key_ScrollLock;         // 91
         // 92 - 96 are OEM-specific
         // 97 - 9F are undefined
         list[VK_LSHIFT]              = Qt::Key::Key_Shift;              // A0
         list[VK_RSHIFT]              = Qt::Key::Key_Shift;              // A1
         list[VK_LCONTROL]            = Qt::Key::Key_Control;            // A2
         list[VK_RCONTROL]            = Qt::Key::Key_Control;            // A3
         list[VK_LMENU]               = Qt::Key::Key_Alt;                // A4
         list[VK_RMENU]               = Qt::Key::Key_Alt;                // A5
         list[VK_BROWSER_BACK]        = Qt::Key::Key_Back;               // A6
         list[VK_BROWSER_FORWARD]     = Qt::Key::Key_Forward;     // A7
         list[VK_BROWSER_REFRESH]     = Qt::Key::Key_Refresh;     // A8
         list[VK_BROWSER_STOP]        = Qt::Key::Key_Stop;        // A9
         list[VK_BROWSER_SEARCH]      = Qt::Key::Key_Search;      // AA
         list[VK_BROWSER_FAVORITES]   = Qt::Key::Key_Favorites;   // AB
         list[VK_BROWSER_HOME]        = Qt::Key::Key_Home;        // AC
         list[VK_VOLUME_MUTE]         = Qt::Key::Key_VolumeMute;         // AD
         list[VK_VOLUME_DOWN]         = Qt::Key::Key_VolumeDown;         // AE
         list[VK_VOLUME_UP]           = Qt::Key::Key_VolumeUp;           // AF
         list[VK_MEDIA_NEXT_TRACK]    = Qt::Key::Key_MediaNext;     // B0
         list[VK_MEDIA_PREV_TRACK]    = Qt::Key::Key_MediaPrevious; // B1
         list[VK_MEDIA_STOP]          = Qt::Key::Key_MediaStop;     // B2
         list[VK_MEDIA_PLAY_PAUSE]    = Qt::Key::Key_MediaPlay;     // B3 // media play/pause
         list[VK_LAUNCH_MAIL]         = Qt::Key::Key_LaunchMail;         // B4
         list[VK_LAUNCH_MEDIA_SELECT] = Qt::Key::Key_LaunchMedia; // B5
         list[VK_LAUNCH_APP1]         = Qt::Key::Key_Launch0;         // B6
         list[VK_LAUNCH_APP2]         = Qt::Key::Key_Launch1;         // B7
         list[VK_OEM_1]               = is_glyph;               // BA
         list[VK_OEM_PLUS]            = is_glyph;            // BB
         list[VK_OEM_COMMA]           = is_glyph;           // BC
         list[VK_OEM_MINUS]           = is_glyph;           // BD
         list[VK_OEM_PERIOD]          = is_glyph;          // BE
         list[VK_OEM_2]               = is_glyph;               // BF
         list[VK_OEM_3]               = is_glyph;               // C0
         // C1 - D7 are reserved
         // D8 - DA are undefined
         list[VK_OEM_4]               = is_glyph;               // DB
         list[VK_OEM_5]               = is_glyph;               // DC
         list[VK_OEM_6]               = is_glyph;               // DD
         list[VK_OEM_7]               = is_glyph;               // DE
         list[VK_OEM_8]               = is_glyph;               // DF
         // E0 is reserved
         // E1 is undefined
         // E2 is VK_OEM_102
         // E3 is undefined
         // E4 is undefined
         // E5 is VK_PROCESSKEY
         // E6 is undefined
         // E7 is VK_PACKET
         // E8 - F5 are undefined
         // F6 is VK_ATTN
         // F7 is VK_CRSEL
         // F8 is VK_EXSEL
         // F9 is VK_EREOF
         list[VK_PLAY]                = Qt::Key::Key_Play;                // FA
         list[VK_ZOOM]                = Qt::Key::Key_Zoom;                // FB
         // FC is VK_NONAME
         // FD is VK_PA1
         list[VK_OEM_CLEAR]           = Qt::Key::Key_Clear;           // FE
         return list;
      })();
   }
}

namespace cobb::qt {
   key::key(Qt::Key code, key_side side) {
      this->code = code;
      this->side = side;
      this->glyph.clear();
      this->update_native_data();
   }
   key::key(QChar c) {
      this->code  = (Qt::Key)0;
      this->side  = key_side::none;
      this->glyph = c;
      this->update_native_data();
   }

   bool key::is_modifier_key() const noexcept {
      switch (this->code) {
         case Qt::Key::Key_Alt:
         case Qt::Key::Key_Control:
         case Qt::Key::Key_Shift:
            return true;
      }
      switch (this->native.vk) {
         case VK_MENU: // Alt
         case VK_LMENU:
         case VK_RMENU:
         case VK_CONTROL:
         case VK_LCONTROL:
         case VK_RCONTROL:
         case VK_SHIFT:
         case VK_LSHIFT:
         case VK_RSHIFT:
            return true;
      }
      return false;
   }

   QString key::toString(bool localize) const {
      if (!this->glyph.isEmpty())
         return this->glyph;
      {
         //
         // Attempt to get the key name from OS-level APIs first.
         //
         WCHAR buffer[20];
         auto len = GetKeyNameTextW((LONG)this->native.scan << 16, buffer, std::extent_v<decltype(buffer)>);
         if (len) {
            if constexpr (std::is_same_v<TCHAR, WCHAR>) {
               return QString::fromUtf16((const char16_t*)buffer, len);
            } else {
               return QString::fromLocal8Bit((const char*)buffer, len);
            }
         }/* else {
            #if _DEBUG
               auto err  = GetLastError();
               auto scan = MapVirtualKey(this->native.vk, 4);
               qDebug("cobb::qt::key::toString: failed; VK %u, scan %u, WinAPI scan %u, last error %08X", this->native.vk, this->native.scan, scan, err);
            #endif
         }*/
      }
      //
      // If OS-level APIs failed, fall back to Qt.
      //
      switch (this->code) {
         case (Qt::Key)0:
         case Qt::Key::Key_unknown:
            break;
         case Qt::Key::Key_Alt:
            return localize ? QCoreApplication::translate("QShortcut", "Alt") : "Alt";
         case Qt::Key::Key_Control:
            return localize ? QCoreApplication::translate("QShortcut", "Ctrl") : "Ctrl";
         case Qt::Key::Key_Shift:
            return localize ? QCoreApplication::translate("QShortcut", "Shift") : "Shift";
         case Qt::Key::Key_Meta:
            return localize ? QCoreApplication::translate("QShortcut", "Meta") : "Meta";
         default:
            //
            // QKeySequence can't handle sequences consisting only of modifier keys, and will 
            // return garbage bytes as a string if you try. We need to pre-filter those.
            //
            return QKeySequence(this->code).toString();
      }
      return QString();
   }

   void key::update_native_data() {
      auto prior = this->native.vk;
      this->native.vk = 0;
      if (this->code != (Qt::Key)0 && this->code != Qt::Key::Key_unknown) {
         auto& list = mappings::vk_to_qt;
         for (size_t i = 0; i < list.size(); ++i) {
            if (list[i] == this->code) {
               this->native.vk = i;
               break;
            }
         }
      }
      if (!this->native.vk && !this->glyph.isEmpty()) {
         auto code = VkKeyScanW(this->glyph[0].unicode());
         if (code != 0xFFFF && (code & 0xFE00) == 0) { // known key, and no modifier keys (other than Shift for caps) needed
            this->native.vk = code & 0xFF;
         }
      }
      if (this->native.vk == 0) {
         this->native.scan = 0;
         return;
      }
      //
      if (this->side != key_side::none) {
         switch (this->native.vk) {
            case VK_CONTROL:
               this->native.vk = (this->side == key_side::left) ? VK_LCONTROL : VK_RCONTROL;
               break;
            case VK_MENU: // alt
               this->native.vk = (this->side == key_side::left) ? VK_LMENU : VK_RMENU;
               break;
            case VK_SHIFT:
               this->native.vk = (this->side == key_side::left) ? VK_LSHIFT : VK_RSHIFT;
               break;
         }
      }
      //
      this->native.scan = MapVirtualKeyW(this->native.vk, 4);
      if (this->native.scan == 0)
         this->native.scan = MapVirtualKeyW(this->native.vk, MAPVK_VK_TO_VSC);
      //
      if (this->code == (Qt::Key)0) {
         if (this->native.vk < mappings::vk_to_qt.size())
            this->code = mappings::vk_to_qt[this->native.vk];
      }
   }

   /*static*/ key key::from_windows_vk(int vk) {
      if (vk < 0)
         return key();
      key result;
      result.native.vk   = vk;
      result.code        = mappings::vk_to_qt[vk];
      result.native.scan = MapVirtualKeyW(vk, 4);
      if (result.native.scan == 0)
         result.native.scan = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
      //
      switch (vk) {
         case VK_LCONTROL:
         case VK_LMENU:
         case VK_LSHIFT:
         case VK_LWIN:
            result.side = key_side::left;
            break;
         case VK_RCONTROL:
         case VK_RMENU:
         case VK_RSHIFT:
         case VK_RWIN:
            result.side = key_side::right;
            break;
      }
      if (result.code == (Qt::Key)0) {
         WCHAR buffer[20];
         auto len = GetKeyNameTextW((LONG)result.native.scan << 16, buffer, std::extent_v<decltype(buffer)>);
         if (len) {
            if constexpr (std::is_same_v<TCHAR, WCHAR>) {
               result.glyph = QString::fromUtf16((const char16_t*)buffer, len);
            } else {
               result.glyph = QString::fromLocal8Bit((const char*)buffer, len);
            }
         }/* else {
            #if _DEBUG
               auto err = GetLastError();
            #endif
         }*/
      }
      return result;
   }
   /*static*/ key key::from_qt_event(const QKeyEvent* event) {
      key result;
      result.native.vk   = event->nativeVirtualKey();
      result.native.scan = event->nativeScanCode();
      result.code        = (Qt::Key)event->key();
      //
      if (result.native.scan == 0)
         result.native.scan = MapVirtualKeyW(result.native.vk, MAPVK_VK_TO_VSC);
      switch (result.native.vk) {
         case VK_LCONTROL:
         case VK_LMENU:
         case VK_LSHIFT:
         case VK_LWIN:
            result.side = key_side::left;
            break;
         case VK_RCONTROL:
         case VK_RMENU:
         case VK_RSHIFT:
         case VK_RWIN:
            result.side = key_side::right;
            break;
      }
      if (result.code == (Qt::Key)0) {
         result.glyph = event->text();
      }
      //
      return result;
   }
}