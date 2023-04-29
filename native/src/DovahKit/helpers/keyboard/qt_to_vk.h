#pragma once
#include <array>
#include <Qt>
#include "./virtual_key.h"

namespace cobb::keyboard {
   namespace impl {
      constexpr const auto vk_to_qt_key = []() {
         constexpr auto unknown = Qt::Key::Key_unknown;
         constexpr auto is_glyph = (Qt::Key)0;
         
         std::array<Qt::Key, 256> list = {};
         for (auto& v : list)
            v = unknown;
         
         list[(size_t)virtual_key::add]              = Qt::Key_Plus;
         list[(size_t)virtual_key::alt]              = Qt::Key_Alt;
         list[(size_t)virtual_key::alt_l]            = Qt::Key_Alt;
         list[(size_t)virtual_key::alt_r]            = Qt::Key_Alt;
         list[(size_t)virtual_key::apps]             = Qt::Key_Menu;
         list[(size_t)virtual_key::arrow_down]       = Qt::Key_Down;
         list[(size_t)virtual_key::arrow_left]       = Qt::Key_Left;
         list[(size_t)virtual_key::arrow_right]      = Qt::Key_Right;
         list[(size_t)virtual_key::arrow_up]         = Qt::Key_Up;
         list[(size_t)virtual_key::backspace]        = Qt::Key_Backspace;
         list[(size_t)virtual_key::browser_back]     = Qt::Key_Back;
         list[(size_t)virtual_key::browser_favs]     = Qt::Key_Favorites;
         list[(size_t)virtual_key::browser_forward]  = Qt::Key_Forward;
         list[(size_t)virtual_key::browser_home]     = Qt::Key_Home;
         list[(size_t)virtual_key::browser_refresh]  = Qt::Key_Refresh;
         list[(size_t)virtual_key::browser_search]   = Qt::Key_Search;
         list[(size_t)virtual_key::browser_stop]     = Qt::Key_Stop;
         list[(size_t)virtual_key::cancel]           = Qt::Key_Cancel;
         list[(size_t)virtual_key::caps_lock]        = Qt::Key_CapsLock;
         list[(size_t)virtual_key::clear]            = Qt::Key_Clear;
         list[(size_t)virtual_key::control]          = Qt::Key_Control;
         list[(size_t)virtual_key::control_l]        = Qt::Key_Control;
         list[(size_t)virtual_key::control_r]        = Qt::Key_Control;
         list[(size_t)virtual_key::del]              = Qt::Key_Delete;
         list[(size_t)virtual_key::divide]           = Qt::Key_Slash;
         list[(size_t)virtual_key::end]              = Qt::Key_End;
         list[(size_t)virtual_key::escape]           = Qt::Key_Escape;
         list[(size_t)virtual_key::help]             = Qt::Key_Help;
         list[(size_t)virtual_key::home]             = Qt::Key_Home;
         list[(size_t)virtual_key::ime_mode_change]  = Qt::Key_Mode_switch;
         list[(size_t)virtual_key::insert]           = Qt::Key_Insert;
         list[(size_t)virtual_key::launch_app_1]     = Qt::Key_Launch0;
         list[(size_t)virtual_key::launch_app_2]     = Qt::Key_Launch1;
         list[(size_t)virtual_key::launch_mail]      = Qt::Key_LaunchMail;
         list[(size_t)virtual_key::launch_media]     = Qt::Key_LaunchMedia;
         list[(size_t)virtual_key::media_next_track] = Qt::Key_MediaNext;
         list[(size_t)virtual_key::media_play_pause] = Qt::Key_MediaPlay;
         list[(size_t)virtual_key::media_prev_track] = Qt::Key_MediaPrevious;
         list[(size_t)virtual_key::media_stop]       = Qt::Key_MediaStop;
         list[(size_t)virtual_key::multiply]         = Qt::Key_Asterisk;
         list[(size_t)virtual_key::num_lock]         = Qt::Key_NumLock;
         list[(size_t)virtual_key::oem_clear]        = Qt::Key_Clear;
         list[(size_t)virtual_key::page_down]        = Qt::Key_PageDown;
         list[(size_t)virtual_key::page_up]          = Qt::Key_PageUp;
         list[(size_t)virtual_key::pause]            = Qt::Key_Pause;
         list[(size_t)virtual_key::play]             = Qt::Key_Play;
         list[(size_t)virtual_key::print_screen]     = Qt::Key_Print;
         list[(size_t)virtual_key::return_]          = Qt::Key_Return;
         list[(size_t)virtual_key::scroll_lock]      = Qt::Key_ScrollLock;
         list[(size_t)virtual_key::select]           = Qt::Key_Select;
         list[(size_t)virtual_key::shift]            = Qt::Key_Shift;
         list[(size_t)virtual_key::shift_l]          = Qt::Key_Shift;
         list[(size_t)virtual_key::shift_r]          = Qt::Key_Shift;
         list[(size_t)virtual_key::sleep]            = Qt::Key_Sleep;
         list[(size_t)virtual_key::space]            = Qt::Key_Space;
         list[(size_t)virtual_key::subtract]         = Qt::Key_Minus;
         list[(size_t)virtual_key::tab]              = Qt::Key_Tab;
         list[(size_t)virtual_key::volume_down]      = Qt::Key_VolumeDown;
         list[(size_t)virtual_key::volume_mute]      = Qt::Key_VolumeMute;
         list[(size_t)virtual_key::volume_up]        = Qt::Key_VolumeUp;
         list[(size_t)virtual_key::windows_l]        = Qt::Key_Meta;
         list[(size_t)virtual_key::windows_l]        = Qt::Key_Meta;
         list[(size_t)virtual_key::zoom]             = Qt::Key_Zoom;

         for (size_t i = 0; i < 10; ++i) {
            list[(size_t)virtual_key::ascii_0  + i] = (Qt::Key)(Qt::Key_0 + i);
            list[(size_t)virtual_key::numpad_0 + i] = (Qt::Key)(Qt::Key_0 + i);
         }
         for (size_t i = 0; i < 26; ++i) {
            list[(size_t)virtual_key::ascii_a + i] = (Qt::Key)(Qt::Key_A + i);
         }
         for (size_t i = 0; i < 24; ++i) {
            list[(size_t)virtual_key::f1 + i] = (Qt::Key)(Qt::Key_F1 + i);
         }

         list[(size_t)virtual_key::oem_char_1]   = is_glyph;
         list[(size_t)virtual_key::oem_char_2]   = is_glyph;
         list[(size_t)virtual_key::oem_char_3]   = is_glyph;
         list[(size_t)virtual_key::oem_char_4]   = is_glyph;
         list[(size_t)virtual_key::oem_char_5]   = is_glyph;
         list[(size_t)virtual_key::oem_char_6]   = is_glyph;
         list[(size_t)virtual_key::oem_char_7]   = is_glyph;
         list[(size_t)virtual_key::oem_char_8]   = is_glyph;

         list[(size_t)virtual_key::oem_comma]  = is_glyph;
         list[(size_t)virtual_key::oem_minus]  = is_glyph;
         list[(size_t)virtual_key::oem_period] = is_glyph;
         list[(size_t)virtual_key::oem_plus]   = is_glyph;

         return list;
      }();
   }

   constexpr virtual_key qt_to_vk(Qt::Key qc) {
      if (qc == (Qt::Key)0 || qc == Qt::Key::Key_unknown)
         return virtual_key::none;

      const auto& list = impl::vk_to_qt_key;
      for (size_t vk = 0; vk < list.size(); ++vk)
         if (list[vk] == qc)
            return (virtual_key)vk;

      return virtual_key::none;
   }
}