#pragma once
#include <array>
#include <string>
#include <vector>
#include <QDebug>
#include "helpers/keyboard/virtual_key.h"
#include "../enums/range_input_axes.h"
#include "../enums/range_input_control.h"
#include "../inputs/button.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput::algorithms {
   namespace impl::input_sequence_stringification {
      using group      = input_sequence::group;
      using group_type = input_sequence::group_type;

      struct _name_to_xinput_button {
         const char* name;
         const char* name_formal;
         inputs::xinput_button button;
      };
      //
      constexpr const auto xinput_button_names = std::array{
         _name_to_xinput_button{ "a",  "A",  inputs::xinput_button::a},
         _name_to_xinput_button{ "b",  "B",  inputs::xinput_button::b },
         _name_to_xinput_button{ "x",  "X",  inputs::xinput_button::x },
         _name_to_xinput_button{ "y",  "Y",  inputs::xinput_button::y },
         _name_to_xinput_button{ "lb", "LB", inputs::xinput_button::lb },
         _name_to_xinput_button{ "rb", "RB", inputs::xinput_button::rb },
         _name_to_xinput_button{ "ls", "LS", inputs::xinput_button::ls },
         _name_to_xinput_button{ "rs", "RS", inputs::xinput_button::rs },
         _name_to_xinput_button{ "lt", "LT", inputs::xinput_button::lt },
         _name_to_xinput_button{ "rt", "RT", inputs::xinput_button::rt },
         _name_to_xinput_button{ "start",       "Start",       inputs::xinput_button::start },
         _name_to_xinput_button{ "back",        "Back",        inputs::xinput_button::back },
         _name_to_xinput_button{ "d-pad up",    "D-Pad Up",    inputs::xinput_button::d_pad_up },
         _name_to_xinput_button{ "d-pad down",  "D-Pad Down",  inputs::xinput_button::d_pad_down },
         _name_to_xinput_button{ "d-pad left",  "D-Pad Left",  inputs::xinput_button::d_pad_left },
         _name_to_xinput_button{ "d-pad right", "D-Pad Right", inputs::xinput_button::d_pad_right },
      };

      struct _name_to_special_key {
         const char* name;
         const char* name_formal;
         cobb::keyboard::virtual_key code;
      };
      //
      constexpr const auto special_key_names = std::array{
         _name_to_special_key{ "alt",         "Alt",         cobb::keyboard::virtual_key::alt },
         _name_to_special_key{ "caps lock",   "Caps Lock",   cobb::keyboard::virtual_key::caps_lock },
         _name_to_special_key{ "ctrl",        "Ctrl",        cobb::keyboard::virtual_key::ctrl },
         _name_to_special_key{ "del",         "Del",         cobb::keyboard::virtual_key::del },
         _name_to_special_key{ "end",         "End",         cobb::keyboard::virtual_key::end },
         _name_to_special_key{ "enter",       "Enter",       cobb::keyboard::virtual_key::return_ },
         _name_to_special_key{ "home",        "Home",        cobb::keyboard::virtual_key::home },
         _name_to_special_key{ "num lock",    "Num Lock",    cobb::keyboard::virtual_key::num_lock },
         _name_to_special_key{ "page down",   "Page Down",   cobb::keyboard::virtual_key::page_down },
         _name_to_special_key{ "page up",     "Page Up",     cobb::keyboard::virtual_key::page_up },
         _name_to_special_key{ "prtscrn",     "PrtScrn",     cobb::keyboard::virtual_key::print_screen },
         _name_to_special_key{ "scroll lock", "Scroll Lock", cobb::keyboard::virtual_key::scroll_lock },
         _name_to_special_key{ "shift",       "Shift",       cobb::keyboard::virtual_key::shift },
         _name_to_special_key{ "space",       "Space",       cobb::keyboard::virtual_key::space },
         _name_to_special_key{ "tab",         "Tab",         cobb::keyboard::virtual_key::tab },
      };

      struct _name_to_directional_control {
         const char* name;
         const char* name_formal;
         range_input_control control = range_input_control::none;
         range_input_axes    axes    = range_input_axes::all;

         constexpr _name_to_directional_control(const char* a, const char* b, range_input_control c) : name(a), name_formal(b), control(c) {}
         constexpr _name_to_directional_control(const char* a, const char* b, range_input_control c, range_input_axes d) : name(a), name_formal(b), control(c), axes(d) {}
      };
      //
      constexpr const auto range_control_names = std::array{
         _name_to_directional_control{ "mouse move",    "Mouse Move",    range_input_control::mouse_move },
         _name_to_directional_control{ "left stick",    "Left Stick",    range_input_control::xinput_ls },
         _name_to_directional_control{ "right stick",   "Right Stick",   range_input_control::xinput_rs },
         _name_to_directional_control{ "left trigger",  "Left Trigger",  range_input_control::xinput_lt },
         _name_to_directional_control{ "right trigger", "Right Trigger", range_input_control::xinput_rt },
         _name_to_directional_control{ "mouse move x",  "Mouse Move X",  range_input_control::mouse_move, range_input_axes::x },
         _name_to_directional_control{ "mouse move y",  "Mouse Move Y",  range_input_control::mouse_move, range_input_axes::y },
         _name_to_directional_control{ "left stick x",  "Left Stick X",  range_input_control::xinput_ls,  range_input_axes::x },
         _name_to_directional_control{ "left stick y",  "Left Stick Y",  range_input_control::xinput_ls,  range_input_axes::y },
         _name_to_directional_control{ "right stick x", "Right Stick X", range_input_control::xinput_rs,  range_input_axes::x },
         _name_to_directional_control{ "right stick y", "Right Stick Y", range_input_control::xinput_rs,  range_input_axes::y },
      };
   }

   #pragma region To string
   constexpr void input_sequence_to_string(const input_sequence& seq, std::string& out) {
      using namespace impl::input_sequence_stringification;

      out.clear();
      if (!seq.root)
         return;

      auto recurse = [](const input_sequence::group& current, std::string& out, auto& recurse) constexpr -> void {
         if (current.type == group_type::single_control) {
            switch (current.button.mouse) {
               case Qt::MouseButton::LeftButton:
                  out += "LMB";
                  return;
               case Qt::MouseButton::RightButton:
                  out += "RMB";
                  return;
               case Qt::MouseButton::MiddleButton:
                  out += "MMB";
                  return;
            }
            if (current.button.gamepad != inputs::xinput_button::none) {
               for (const auto& known : xinput_button_names) {
                  if (known.button == current.button.gamepad) {
                     out += known.name_formal;
                     return;
                  }
               }
            }
            for (const auto& known : special_key_names) {
               if (current.button.key.vk == known.code) {
                  out += known.name_formal;
                  return;
               }
            }

            out += current.button.key.get_key_name_ansi();
            return;
         }
         switch (current.type) {
            case group_type::concurrent_ordered:   out += '['; break;
            case group_type::concurrent_unordered: out += '('; break;
            case group_type::separated_ordered:    out += '<'; break;
         }
         bool is_first = true;
         for (const auto* child : current.children) {
            if (is_first) {
               is_first = false;
            } else {
               out += " + ";
            }
            recurse(*child, out, recurse);
         }
         switch (current.type) {
            case group_type::concurrent_ordered:   out += ']'; break;
            case group_type::concurrent_unordered: out += ')'; break;
            case group_type::separated_ordered:    out += '>'; break;
         }
      };
      recurse(*seq.root, out, recurse);

      if (seq.has_range_requirement()) {
         out += " :: ";
         for (const auto& known : range_control_names) {
            if (seq.range.control == known.control && seq.range.axes == known.axes) {
               out += known.name_formal;
               break;
            }
         }
      }
   }
   #pragma endregion

   #pragma region From string
   constexpr input_sequence input_sequence_from_string(const std::string& str, bool gamepad = false) {
      using namespace impl::input_sequence_stringification;

      input_sequence out;

      std::vector<group*> nesting;

      size_t i = 0;
      for (; i < str.size(); ++i) {
         const char c = str[i];

         if (c == ':') {
            if (i + 1 < str.size()) {
               char d = str[i + 1];
               if (d == ':') {
                  i += 2;

                  std::string dir_name;
                  for (size_t j = i; j < str.size(); ++j) {
                     d = str[j];
                     switch (d) {
                        case '[':
                        case '(':
                        case '<':
                        case '>':
                        case ')':
                        case ']':
                        case '+':
                           if (std::is_constant_evaluated()) {
                              throw;
                           } else {
                              qDebug("input_sequence::debug_from_string: unexpected %c (expected directional input only) at position %d in: '%s'", d, i, str.c_str());
                              __debugbreak();
                           }
                           break;
                     }
                     dir_name += d;
                  }

                  {  // Trim
                     size_t m = dir_name.find_first_not_of(" \r\n\t");
                     size_t n = dir_name.find_last_not_of(" \r\n\t");
                     if (n != std::string::npos) {
                        dir_name.resize(n + 1);
                        dir_name.erase(0, m);
                     }
                  }
                  for (size_t i = 0; i < dir_name.size(); ++i) { // to lower (ASCII)
                     auto& c = dir_name[i];
                     if (c >= 'A' && c <= 'Z')
                        c |= 0x20;
                  }

                  bool found = false;
                  for (const auto& known : range_control_names) {
                     if (dir_name == known.name) {
                        out.range.control = known.control;
                        out.range.axes    = known.axes;
                        found = true;
                        break;
                     }
                  }
                  if (!found) {
                     if (std::is_constant_evaluated()) {
                        throw;
                     } else {
                        qDebug("input_sequence::debug_from_string: unrecognized range control name in: '%s'", str.c_str());
                        __debugbreak();
                     }
                  }
                  break;
               }
            }
         }

         auto type  = group_type::single_control;
         bool close = false;

         switch (c) {
            case '[': type = group_type::concurrent_ordered;   break;
            case '(': type = group_type::concurrent_unordered; break;
            case '<': type = group_type::separated_ordered;    break;
            case ']': type = group_type::concurrent_ordered;   close = true; break;
            case ')': type = group_type::concurrent_unordered; close = true; break;
            case '>': type = group_type::separated_ordered;    close = true; break;
            case '+':
               if (std::is_constant_evaluated()) {
                  throw;
               } else {
                  qDebug("input_sequence::debug_from_string: unexpected + at position %d in: '%s'", i, str.c_str());
                  __debugbreak();
               }
               [[fallthrough]];
            case ' ':
               continue;
         }
         if (close) {
            if (nesting.empty()) {
               if (std::is_constant_evaluated()) {
                  throw;
               } else {
                  qDebug("input_sequence::debug_from_string:: unexpected closing delimiter %c at position %d", c, i);
                  __debugbreak();
               }
            }
            nesting.pop_back();

            //
            // Advance to next closing delimiter, or past next '+':
            //
            for (++i; i < str.size(); ++i) {
               const char d = str[i];
               if (d == '+') {
                  break;
               }
               if (d == ':') { // "::" is the separator for range constraints
                  if (i + 1 < str.size() && str[i + 1] == ':') {
                     --i;
                     break;
                  }
               }
               if (d == ']' || d == ')' || d == '>') {
                  --i;
                  break;
               }
               if (d == ' ' || d == '\t' || d == '\n' || d == '\r') {
                  continue;
               }
               //
               // Unexpected character.
               //
               if (std::is_constant_evaluated()) {
                  throw;
               } else {
                  qDebug("input_sequence::debug_from_string:: unexpected character %c at position %d", d, i);
                  __debugbreak();
               }
            }

            continue;
         }

         auto* child = new group;
         child->type = type;
         //
         if (nesting.empty()) {
            out.root = child;
         } else {
            auto* parent = nesting.back();
            assert(parent);
            assert(parent->type != group_type::single_control);

            parent->children.push_back(child);
         }

         if (type == group_type::single_control) {
            //
            // Advance to next ending delimiter, or to next '+'.
            //
            assert(c != ' ');
            std::string name;
            name += c;
            for (++i; i < str.size(); ++i) {
               const char d = str[i];
               if (d == '+') {
                  break;
               }
               if (d == ':') { // "::" is the separator for range constraints
                  if (i + 1 < str.size() && str[i + 1] == ':') {
                     --i;
                     break;
                  }
               }
               if (d == ']' || d == ')' || d == '>') {
                  --i;
                  break;
               }
               name += d;
            }

            // trim trailing whitespace:
            size_t j;
            for (j = name.size() - 1; j > 0; --j)
               if (name[j] != ' ')
                  break;
            name.resize(j + 1);

            if (gamepad) {
               //
               // Gamepad
               //
               auto n = name;
               for (size_t i = 0; i < n.size(); ++i) { // to lower (ASCII)
                  auto& c = n[i];
                  if (c >= 'A' && c <= 'Z')
                     c |= 0x20;
               }
               {
                  bool found = false;
                  for (const auto& known : xinput_button_names) {
                     if (n == known.name) {
                        child->button = inputs::button{ .gamepad = known.button };
                        found = true;
                        break;
                     }
                  }
                  if (!found) {
                     if (std::is_constant_evaluated()) {
                        throw;
                     } else {
                        qDebug("input_sequence::debug_from_string: unrecognized gamepad button name: %s", n.c_str());
                     }
                  }
               }
            } else {
               //
               // Keyboard and mouse
               //
               if (name.size() == 1) {
                  child->button = inputs::button{ .key = cobb::keyboard::key(name[0])};
               } else {
                  auto n = name;
                  for (size_t i = 0; i < n.size(); ++i) { // to lower (ASCII)
                     auto& c = n[i];
                     if (c >= 'A' && c <= 'Z')
                        c |= 0x20;
                  }
                  
                  bool found = false;
                  for (const auto& known : special_key_names) {
                     if (n == known.name) {
                        child->button = inputs::button{ .key = cobb::keyboard::key(known.code) };
                        found = true;
                        break;
                     }
                  }
                  if (!found) {
                     if (n == "lmb") {
                        child->button = inputs::button{ .mouse = Qt::MouseButton::LeftButton };
                     } else if (n == "rmb") {
                        child->button = inputs::button{ .mouse = Qt::MouseButton::RightButton };
                     } else if (n == "mmb") {
                        child->button = inputs::button{ .mouse = Qt::MouseButton::MiddleButton };
                     } else {
                        if (std::is_constant_evaluated()) {
                           throw;
                        } else {
                           qDebug("input_sequence::debug_from_string: unrecognized key name: %s", n.c_str());
                        }
                     }
                  }
               }
            }
            continue;
         } else {
            nesting.push_back(child);
         }
      }

      return out;
   }
   #pragma endregion
}