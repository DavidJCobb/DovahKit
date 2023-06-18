#pragma once
#include "./button.h"
#include <bit>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput2::inputs {
   namespace impl::_button {
      enum class serialized_type : uint8_t {
         none,
         keyboard,
         mouse,
         gamepad,
      };

      constexpr const auto xinput_button_to_serialized_index = []() consteval {
         using enum xinput_button;
         return std::array{
            none,
            a,
            b,
            x,
            y,
            start,
            back,
            d_pad_up,
            d_pad_down,
            d_pad_left,
            d_pad_right,
            ls,
            rs,
            lb,
            rb,
            lt,
            rt,
         };
      }();
      constexpr const auto xinput_button_bitcount = std::bit_width(xinput_button_to_serialized_index.size());

      constexpr const size_t xinput_button_to_bits(xinput_button b) {
         const auto& list = xinput_button_to_serialized_index;
         for (size_t i = 0; i < list.size(); ++i)
            if (list[i] == b)
               return i;
         return 0;
      }
      constexpr const xinput_button xinput_button_from_bits(uint8_t b) {
         if (b < xinput_button_to_serialized_index.size())
            return xinput_button_to_serialized_index[b];
         return xinput_button::none;
      }
   }
}

#include "helpers/bitstreams/enum_serialization_options.h"
template<> struct cobb::bitstreams::enum_serialization_options<dovahkit::subsystems::worldinput2::inputs::impl::_button::serialized_type> {
   using value_type = dovahkit::subsystems::worldinput2::inputs::impl::_button::serialized_type;

   static constexpr const size_t bitcount = 2;
   static constexpr const auto   valid_values = []() {
      return std::array{ value_type::none, value_type::keyboard, value_type::mouse, value_type::gamepad };
   }();
};

namespace dovahkit::subsystems::worldinput2::inputs {
   constexpr void button::stream(cobb::bitstreams::reader& s) {
      using serialized_type = impl::_button::serialized_type;

      serialized_type type = {};
      s.stream(type);

      bool presence;
      
      this->key     = {};
      this->mouse   = Qt::MouseButton::NoButton;
      this->gamepad = xinput_button::none;
      switch (type) {
         case serialized_type::none:
            break;
         case serialized_type::keyboard:
            {
               s.stream_bits(8, this->key.vk);

               s.stream(presence);
               if (presence)
                  s.stream_bits(32, this->key.scan_code);

               s.stream(presence);
               if (presence)
                  s.stream_bits(32, this->key.unicode);
            }
            break;
         case serialized_type::mouse:
            s.stream_bits(32, this->mouse);
            break;
         case serialized_type::gamepad:
            {
               size_t b = {};
               s.stream_bits(impl::_button::xinput_button_bitcount, b);
               this->gamepad = impl::_button::xinput_button_from_bits(b);
            }
            break;
      }
   }
   constexpr void button::stream(cobb::bitstreams::writer& s) const {
      using serialized_type = impl::_button::serialized_type;

      // Right now, we don't have any systems for handling input given a key's 
      // physical position on the keyboard; we can only process virtual keys. 
      // Among other limitations, this means that we can't tell an AltGr key 
      // apart from (Alt + Ctrl). As such, we'll only serialize VKs for now.
      constexpr const bool allow_saving_detailed_keys = false;

      serialized_type type = serialized_type::none;
      //
      if (!this->key.empty()) {
         type = serialized_type::keyboard;
      } else if (this->mouse != Qt::MouseButton::NoButton) {
         type = serialized_type::mouse;
      } else if (this->gamepad != xinput_button::none) {
         type = serialized_type::gamepad;
      }

      s.stream(type);
      switch (type) {
         case serialized_type::none:
            break;
         case serialized_type::keyboard:
            {
               s.stream_bits(8, this->key.vk);

               if constexpr (allow_saving_detailed_keys) {
                  s.stream(this->key.has_scan_code());
                  if (this->key.has_scan_code())
                     s.stream_bits(32, this->key.scan_code);

                  s.stream(this->key.has_unicode());
                  if (this->key.has_unicode())
                     s.stream_bits(32, this->key.unicode);
               } else {
                  s.stream(false); // presence bit for scan code
                  s.stream(false); // presence bit for glyph
               }
            }
            break;
         case serialized_type::mouse:
            s.stream_bits(32, this->mouse);
            break;
         case serialized_type::gamepad:
            {
               s.stream_bits(impl::_button::xinput_button_bitcount, impl::_button::xinput_button_to_bits(this->gamepad));
            }
            break;
      }
   }
}