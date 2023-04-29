#pragma once
#include <cstdint>

namespace cobb::keyboard {
   // Same as the Windows constants, but with better type-safety and no need 
   // for the Windows headers.
   enum class virtual_key : std::uint8_t {
      none = 0,
      //
      mouse_left        = 0x01,
      mouse_right       = 0x02,
      cancel            = 0x03, // Control-break processing
      mouse_middle      = 0x04,
      mouse_x1          = 0x05,
      mouse_x2          = 0x06,
      // 0x07: undefined
      backspace         = 0x08,
      tab               = 0x09,
      // 0x0A: reserved
      // 0x0B: reserved
      clear             = 0x0C,
      return_           = 0x0D,
      // 0x0E: undefined
      // 0x0F: undefined
      shift             = 0x10,
      control           = 0x11, ctrl   = control,
      menu              = 0x12, alt    = menu,
      pause             = 0x13,
      caps_lock         = 0x14,
      ime_kana          = 0x15, // IME Kana mode
      ime_hangul        = 0x15, // IME Hangul mode
      ime_on            = 0x16, // IME On
      ime_junja         = 0x17,
      ime_final         = 0x18,
      ime_hanja         = 0x19,
      ime_kanji         = 0x19,
      ime_off           = 0x1A, // IME Off
      escape            = 0x1B, esc   = escape,
      ime_convert       = 0x1C,
      ime_non_convert   = 0x1D,
      ime_accept        = 0x1E,
      ime_mode_change   = 0x1F,
      space             = 0x20,
      page_up           = 0x21,
      page_down         = 0x22,
      end               = 0x23,
      home              = 0x24,
      arrow_left        = 0x25,
      arrow_up          = 0x26,
      arrow_right       = 0x27,
      arrow_down        = 0x28,
      select            = 0x29,
      print             = 0x2A,
      execute           = 0x2B,
      print_screen      = 0x2C,
      insert            = 0x2D,
      delete_           = 0x2E, del = delete_,
      help              = 0x2F,
      ascii_0           = 0x30,
      ascii_1,
      ascii_2,
      ascii_3,
      ascii_4,
      ascii_5,
      ascii_6,
      ascii_7,
      ascii_8,
      ascii_9,
      // 0x3A: undefined
      // 0x3B: undefined
      // 0x3C: undefined
      // 0x3D: undefined
      // 0x3E: undefined,
      // 0x3F: undefined
      // 0x40: undefined
      ascii_a           = 0x41,
      ascii_b,
      ascii_c,
      ascii_d,
      ascii_e,
      ascii_f,
      ascii_g,
      ascii_h,
      ascii_i,
      ascii_j,
      ascii_k,
      ascii_l,
      ascii_m,
      ascii_n,
      ascii_o,
      ascii_p,
      ascii_q,
      ascii_r,
      ascii_s,
      ascii_t,
      ascii_u,
      ascii_v,
      ascii_w,
      ascii_x,
      ascii_y,
      ascii_z,
      windows_l         = 0x5B, // left-side Windows key (Natural keyboard)
      windows_r         = 0x5C, // right-side Windows key (Natural keyboard)
      apps              = 0x5D, // applications key (Natural keyboard)
      // 0x5E: reserved
      sleep             = 0x5F,
      numpad_0          = 0x60,
      numpad_1          = 0x61,
      numpad_2          = 0x62,
      numpad_3          = 0x63,
      numpad_4          = 0x64,
      numpad_5          = 0x65,
      numpad_6          = 0x66,
      numpad_7          = 0x67,
      numpad_8          = 0x68,
      numpad_9          = 0x69,
      multiply          = 0x6A,
      add               = 0x6B,
      separator         = 0x6C,
      subtract          = 0x6D,
      decimal           = 0x6E,
      divide            = 0x6F,
      f1                = 0x70,
      f2                = 0x71,
      f3                = 0x72,
      f4                = 0x73,
      f5                = 0x74,
      f6                = 0x75,
      f7                = 0x76,
      f8                = 0x77,
      f9                = 0x78,
      f10               = 0x79,
      f11               = 0x7A,
      f12               = 0x7B,
      f13               = 0x7C,
      f14               = 0x7D,
      f15               = 0x7E,
      f16               = 0x7F,
      f17               = 0x80,
      f18               = 0x81,
      f19               = 0x82,
      f20               = 0x83,
      f21               = 0x84,
      f22               = 0x85,
      f23               = 0x86,
      f24               = 0x87,
      // 0x88: unassigned
      // 0x89: unassigned
      // 0x8A: unassigned
      // 0x8B: unassigned
      // 0x8C: unassigned
      // 0x8D: unassigned
      // 0x8E: unassigned
      // 0x8F: unassigned
      num_lock          = 0x90,
      scroll_lock       = 0x91,
      // 0x92: OEM-specific
      // 0x93: OEM-specific
      // 0x94: OEM-specific
      // 0x95: OEM-specific
      // 0x96: OEM-specific
      // 0x97: unassigned
      // 0x98: unassigned
      // 0x99: unassigned
      // 0x9A: unassigned
      // 0x9B: unassigned
      // 0x9C: unassigned
      // 0x9D: unassigned
      // 0x9E: unassigned
      // 0x9F: unassigned
      shift_l           = 0xA0,
      shift_r           = 0xA1,
      control_l         = 0xA2, ctrl_l   = control_l,
      control_r         = 0xA3, ctrl_r   = control_r,
      menu_l            = 0xA4, alt_l   = menu_l,
      menu_r            = 0xA5, alt_r   = menu_r,
      browser_back      = 0xA6,
      browser_forward   = 0xA7,
      browser_refresh   = 0xA8,
      browser_stop      = 0xA9,
      browser_search    = 0xAA,
      browser_favs      = 0xAB, browser_favorites   = browser_favs,
      browser_home      = 0xAC,
      volume_mute       = 0xAD,
      volume_down       = 0xAE,
      volume_up         = 0xAF,
      media_next_track  = 0xB0,
      media_prev_track  = 0xB1,
      media_stop        = 0xB2,
      media_play_pause  = 0xB3,
      launch_mail       = 0xB4,
      launch_media      = 0xB5,
      launch_app_1      = 0xB6,
      launch_app_2      = 0xB7,
      // 0xB8: reserved
      // 0xB9: reserved
      oem_char_1        = 0xBA, us_colon = oem_char_1, us_semicolon = oem_char_1,
      oem_plus          = 0xBB,
      oem_comma         = 0xBC,
      oem_minus         = 0xBD,
      oem_period        = 0xBE,
      oem_char_2        = 0xBF, us_slash = oem_char_2, us_question = oem_char_2,
      oem_char_3        = 0xC0, us_tilde = oem_char_3, us_backtick = oem_char_3,
      // 0xC1: reserved
      // 0xC2: reserved
      // 0xC3: reserved
      // 0xC4: reserved
      // 0xC5: reserved
      // 0xC6: reserved
      // 0xC7: reserved
      // 0xC8: reserved
      // 0xC9: reserved
      // 0xCA: reserved
      // 0xCB: reserved
      // 0xCC: reserved
      // 0xCD: reserved
      // 0xCE: reserved
      // 0xCF: reserved
      // 0xD0: reserved
      // 0xD1: reserved
      // 0xD2: reserved
      // 0xD3: reserved
      // 0xD4: reserved
      // 0xD5: reserved
      // 0xD6: reserved
      // 0xD7: reserved
      // 0xD8: unassigned
      // 0xD9: unassigned
      // 0xDA: unassigned
      oem_char_4        = 0xDB, us_brace_l = oem_char_4, us_bracket_l = oem_char_4,
      oem_char_5        = 0xDC, us_backslash = oem_char_5, us_pipe = oem_char_5,
      oem_char_6        = 0xDD, us_brace_r = oem_char_6, us_bracket_r = oem_char_6,
      oem_char_7        = 0xDE, us_quote = oem_char_7,
      oem_char_8        = 0xDF,
      // 0xE0: resered
      // 0xE1: OEM-specific
      oem_char_102      = 0xE2, us_angle_brackets = oem_char_102, // what, both of them?
      // 0xE3: OEM-specific
      // 0xE4: OEM-specific
      ime_process       = 0xE5,
      // 0xE6: OEM-specific
      unicode_packet    = 0xE7, // Win32: sentinel for Unicode characters passed via a non-keyboard input method
      // 0xE8: unassigned
      // 0xE9: OEM-specific
      // 0xEA: OEM-specific
      // 0xEB: OEM-specific
      // 0xEC: OEM-specific
      // 0xED: OEM-specific
      // 0xEE: OEM-specific
      // 0xEF: OEM-specific
      // 0xF0: OEM-specific
      // 0xF1: OEM-specific
      // 0xF2: OEM-specific
      // 0xF3: OEM-specific
      // 0xF4: OEM-specific
      // 0xF5: OEM-specific
      attn              = 0xF6,
      crsel             = 0xF7,
      exsel             = 0xF8,
      erase_eof         = 0xF9,
      play              = 0xFA,
      zoom              = 0xFB,
      // 0xFC: reserved (VK_NONAME)
      pa1               = 0xFD,
      oem_clear         = 0xFE,
      _unknown          = 0xFF, // sometimes passed by Windows as some kind of sentinel
   };
}