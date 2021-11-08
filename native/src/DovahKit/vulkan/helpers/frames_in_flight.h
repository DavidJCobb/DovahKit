#pragma once
#include <algorithm>
#include <bit>
#include "../config/frames_in_flight.h"

namespace vulkanDK {
   namespace impl {
      template<unsigned int bytecount> struct bytecount_to_int {
         using type = struct _disabled { _disabled() { static_assert(false, "Unsupported bytecount."); } };
      };
      template<> struct bytecount_to_int<1> { using type = uint8_t; };
      template<> struct bytecount_to_int<2> { using type = uint16_t; };
      template<> struct bytecount_to_int<4> { using type = uint32_t; };
      template<> struct bytecount_to_int<8> { using type = uint64_t; };

      template<unsigned int bitcount> inline constexpr unsigned int sized_mask_size = (std::max)((unsigned int)8, std::bit_ceil(bitcount)) / 8;
      template<unsigned int bitcount> using sized_mask_type = bytecount_to_int<sized_mask_size<bitcount>>::type;
   }

   class frames_in_flight_mask {
      public:
         using mask_type = impl::sized_mask_type<config::frames_in_flight_count>;

         static constexpr mask_type all_bits = (config::frames_in_flight_count == sizeof(mask_type)) ? mask_type(-1) : mask_type((1 << config::frames_in_flight_count) - 1);

      public:
         mask_type mask = 0;

         inline bool any_set() const noexcept { return (this->mask & all_bits) != 0; }
         inline void set_all() { this->mask = all_bits; }

         inline bool test(size_t i) const noexcept { return (this->mask & (1 << i)) != 0; }
         inline void set(size_t i) { this->mask |= (1 << i); }
         inline void clear(size_t i) { this->mask &= ~(1 << i); }
   };
}