#pragma once
#include <array>
#include <cstdint>

namespace cobb::color {
   struct rgb_bytes;
   struct rgb_floats;

   struct rgb_bytes {
      union {
         std::array<uint8_t, 3> components = {};
         struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
         };
      };

      constexpr bool operator==(const rgb_bytes&) const;
      constexpr operator rgb_floats() const;
   };

   struct rgb_floats {
      union {
         std::array<float, 3> components = {};
         struct {
            float r;
            float g;
            float b;
         };
      };

      constexpr void clamp();
      constexpr rgb_floats clamped() const;

      rgb_floats sRGB_to_linear() const;
      rgb_floats linear_to_sRGB() const;

      constexpr bool operator==(const rgb_floats&) const;
      constexpr operator rgb_bytes() const;
   };
}

#include "./rgb.inl"