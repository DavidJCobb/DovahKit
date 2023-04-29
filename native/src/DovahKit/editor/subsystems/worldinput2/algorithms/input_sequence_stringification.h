#pragma once
#include <array>
#include <string>
#include <vector>
#include <QDebug>
#include "helpers/keyboard/virtual_key.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::algorithms {
   namespace impl::input_sequence_stringification {
      using group      = input_sequence::group;
      using group_type = input_sequence::group_type;

      struct _name_to_xinput_button {
         const char* name;
         const char* name_formal;
         inputs::xinput_button button;
      };

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
   }

   #pragma region To string
   constexpr void input_sequence_to_string(const input_sequence& seq, std::string& out) {
      out.clear();
      if (!seq.root)
         return;

      auto recurse = [](const input_sequence::group& current, std::string& out, auto& recurse) constexpr -> void {
         using namespace impl::input_sequence_stringification;

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
         for (const auto* child : current.children)
            recurse(*child, out, recurse);
         switch (current.type) {
            case group_type::concurrent_ordered:   out += ']'; break;
            case group_type::concurrent_unordered: out += ')'; break;
            case group_type::separated_ordered:    out += '>'; break;
         }
      };
      recurse(*seq.root, out, recurse);
   }
   #pragma endregion

   #pragma region From string
   extern input_sequence input_sequence_from_string(const std::string& str, bool gamepad = false) {
      using namespace impl::input_sequence_stringification;

      input_sequence out;

      std::vector<group*> nesting;

      size_t i = 0;
      for (; i < str.size(); ++i) {
         const char c = str[i];

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
               auto n = QString(name.c_str()).toLower();
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
                        qDebug("input_sequence::debug_from_string: unrecognized gamepad button name: %s", qUtf8Printable(n));
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
                  auto n = QString(name.c_str()).toLower();
                  
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
                           qDebug("input_sequence::debug_from_string: unrecognized key name: %s", qUtf8Printable(n));
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