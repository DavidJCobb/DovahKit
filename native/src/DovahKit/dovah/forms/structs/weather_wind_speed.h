#pragma once
#include <cstdint>

namespace dovah::loaded_forms::structs {
   struct weather_wind_speed {
      public:
         static constexpr uint8_t pack(float v) {
            int result = v * 10 * 127 + 127;
            if (result <= 0)
               return 0;
            else if (result >= 255)
               return 255;
            return result;
         }
         static constexpr float unpack(uint8_t v) {
            return ((int)v - 127) / 127.0F / 10.0F;
         }

         static constexpr const float minimum = -0.1F;
         static constexpr const float maximum =  0.1F;

         // Can't define this in terms of `unpack` due to C++ standard nonsense. Applies 
         // to both MSVC and GCC; GCC has an explanation in a rejected bug report:
         // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52366
         // 
         // Simply put: static constexpr members of a class can't be defined in terms of 
         // anything that requires the class to be "complete," which supposedly includes 
         // the bodies of static constexpr member functions.
         //
         static constexpr const float step = 1 / 127.0F / 10.0F;

      public:
         uint8_t raw = 0;

      public:
         constexpr weather_wind_speed() {}
         constexpr weather_wind_speed(float f) {
            this->set_float(f);
         }
         explicit constexpr weather_wind_speed(uint8_t raw) : raw(raw) {}

         constexpr operator float() const noexcept {
            return unpack(this->raw);
         }
         constexpr void set_float(float f) {
            this->raw = pack(f);
         }
   };
}