#pragma once
#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include "./bits/all_ones.h"
#include "./bits/replace_most_significant_bits.h"
#include "./type_traits/set_const.h"
#include "./type_traits/strip_enum.h"
#include "./uint_of_size.h"

namespace cobb {

   namespace impl::_bitfield_array {
      template<typename T, typename U> concept can_binary_op = requires(const T& t, const U& u) {
         { t & u };
         { t | u };
         { t ^ u };
      };
   }

   //
   // A bitpacked array capable of storing integral and enum values using a desired bitcount per value. 
   // Note that if the value type is signed (including `int`), one of the bits you allot per value will 
   // be consumed as a sign bit.
   // 
   // By default, the values are packed into bytes. You can change the packing unit by passing your own 
   // unit type.
   //
   template<typename T, size_t Size, size_t BitsPerValue, typename UnitType = uint8_t>
   requires (
      (std::is_integral_v<T> || std::is_enum_v<T>) // value type must be a plain ol' number
   && sizeof(T) <= 8 // must be possible to store the value type in a single integral during get/set operations
   && std::is_integral_v<UnitType>
   && BitsPerValue > 0
   )
   class bitfield_array {
      public:
         using value_type = T;
         using unit_type  = UnitType;
         using size_type  = size_t;
         static constexpr const size_t value_bitcount = BitsPerValue;
         static constexpr const size_t total_bitcount = Size * value_bitcount;
         static constexpr const size_t unit_bitcount  = sizeof(unit_type) * 8;
         static constexpr const size_t unit_count     = (total_bitcount / unit_bitcount) + ((total_bitcount % unit_bitcount) ? 1 : 0);
      protected:
         static constexpr const bool is_signed_value_type = std::is_signed_v<strip_enum_t<value_type>>;

         using underlying_integral_type = uint_of_size<sizeof(value_type)>;

         static constexpr const auto value_mask    = cobb::bits::all_ones<value_bitcount, underlying_integral_type>();
         static constexpr const auto sign_bit_mask = underlying_integral_type(1) << (value_bitcount - 1);

         unit_type units[unit_count] = {};

         static constexpr void _bounds_check(size_t index) {
            if (std::is_constant_evaluated()) {
               if (index >= Size)
                  throw;
            }
         }

         template<bool Const> struct _value_wrapper_impl {
            using owner_type = set_const<bitfield_array, Const>;

            owner_type& target;
            size_t index;

            constexpr _value_wrapper_impl(owner_type& ba, size_t i) : target(ba), index(i) {}

            constexpr _value_wrapper_impl& operator=(const value_type& v) requires (!Const) {
               target.set(index, v);
               return *this;
            }
            constexpr operator value_type() const {
               return target.get(index);
            }

            template<impl::_bitfield_array::can_binary_op<value_type> Other>
            constexpr _value_wrapper_impl& operator|=(const Other& v) {
               this->operator=((value_type)(*this) | v);
               return *this;
            }
            template<impl::_bitfield_array::can_binary_op<value_type> Other>
            constexpr _value_wrapper_impl& operator&=(const Other& v) {
               this->operator=((value_type)(*this) & v);
               return *this;
            }
            template<impl::_bitfield_array::can_binary_op<value_type> Other>
            constexpr _value_wrapper_impl& operator^=(const Other& v) {
               this->operator=((value_type)(*this) ^ v);
               return *this;
            }
            template<typename Other>
            constexpr _value_wrapper_impl& operator+=(const Other& v) {
               this->operator=((value_type)(*this) + v);
               return *this;
            }
            template<typename Other>
            constexpr _value_wrapper_impl& operator-=(const Other& v) {
               this->operator=((value_type)(*this) - v);
               return *this;
            }
            template<typename Other>
            constexpr _value_wrapper_impl& operator*=(const Other& v) {
               this->operator=((value_type)(*this) * v);
               return *this;
            }
            template<typename Other>
            constexpr _value_wrapper_impl& operator/=(const Other& v) {
               this->operator=((value_type)(*this) / v);
               return *this;
            }
            template<typename Other>
            constexpr _value_wrapper_impl& operator%=(const Other& v) {
               this->operator=((value_type)(*this) % v);
               return *this;
            }
         };
         //
         using value_wrapper = _value_wrapper_impl<false>;
         using const_value_wrapper = _value_wrapper_impl<true>;

         static constexpr underlying_integral_type _sign_extend_raw_bits(underlying_integral_type value) {
            if constexpr (is_signed_value_type) {
               if (value & sign_bit_mask) {
                  value = ~value_mask | value;
               }
            }
            return value;
         }

         template<
            typename Self,
            typename SingleChunkFunctor, // void functor(unit_type& unit)
            typename LeadingBitsFunctor, // void functor(unit_type& unit, uint8_t bitshift, uint8_t bitcount, size_t remaining_value_bits)
            typename WholeChunkFunctor,  // void functor(unit_type& unit, uint8_t trailing_bitcount, size_t whole_unit_bits_remaining)
            typename TrailingBitsFunctor // void functor(unit_type& unit, uint8_t bitcount)
         > static constexpr void _access_element(
            Self self,
            size_t index,
            SingleChunkFunctor scf, // Runs if a single element exactly fills a single unit.
            LeadingBitsFunctor lbf, // Handles the first few bits of an element. Receives the unit, bit-offset within the unit, number of bits in this unit that belong to the element, and bits left to read afterward.
            WholeChunkFunctor wcf,  // For large element sizes, handles any whole units spanned by the middle/end of an element. Receives the number of bits that'll go to TrailingBitsFunctor, and the number of whole-unit bits left to read.
            TrailingBitsFunctor tbf // Handles the last few bits of an element, if the element is broken across multiple units.
         ) {
            size_t bitpos  = index * value_bitcount;
            size_t bytepos = bitpos / unit_bitcount;

            if constexpr (unit_bitcount == value_bitcount) {
               scf(self->units[bytepos]);
               return;
            }
            if constexpr ((unit_bitcount % value_bitcount) == 0) {
               //
               // Bits per unit is a multiple of item bitcount, so items will never be 
               // split across multiple units.
               //
               lbf(self->units[bytepos], bitpos % unit_bitcount, value_bitcount, 0);
               return;
            } else {
               size_t  remaining = value_bitcount;
               uint8_t shift     = bitpos % unit_bitcount;
               if (shift) {
                  //
                  // Handle leading unaligned bits first.
                  //
                  uint8_t unaligned_bitcount = unit_bitcount - shift;

                  if (remaining <= unaligned_bitcount) {
                     lbf(self->units[bytepos], shift, remaining, 0);
                     return;
                  } else {
                     remaining -= unaligned_bitcount;
                     lbf(self->units[bytepos], shift, unaligned_bitcount, remaining);
                  }
                  ++bytepos;
               }
               //
               // All operations from here on out will be unit-aligned. Let's handle 
               // whole units, and then any trailing bits.
               //
               uint8_t trailing_bitcount = remaining % unit_bitcount;
               if constexpr (value_bitcount > unit_bitcount) {  // Write whole chunks.
                  size_t rem_wholes = remaining - trailing_bitcount;
                  while (rem_wholes) {
                     wcf(self->units[bytepos++], trailing_bitcount, (rem_wholes -= unit_bitcount));
                  }
               }
               if (trailing_bitcount == 0)
                  return;
               tbf(self->units[bytepos], trailing_bitcount);
            }
         }
         //
         template<typename SingleChunkFunctor, typename LeadingBitsFunctor, typename WholeChunkFunctor, typename TrailingBitsFunctor>
         constexpr void _access_element(
            size_t index,
            SingleChunkFunctor scf,
            LeadingBitsFunctor lbf,
            WholeChunkFunctor wcf,
            TrailingBitsFunctor tbf
         ) {
            _access_element(this, index, scf, lbf, wcf, tbf);
         }
         template<typename SingleChunkFunctor, typename LeadingBitsFunctor, typename WholeChunkFunctor, typename TrailingBitsFunctor>
         constexpr void _access_element(
            size_t index,
            SingleChunkFunctor scf,
            LeadingBitsFunctor lbf,
            WholeChunkFunctor wcf,
            TrailingBitsFunctor tbf
         ) const {
            _access_element(this, index, scf, lbf, wcf, tbf);
         }

      public:
         constexpr bitfield_array() {}

         constexpr size_t size() const noexcept { return Size; }
         constexpr unit_type* data() noexcept { return &this->units[0]; }
         constexpr const unit_type* data() const noexcept { return &this->units[0]; }

         static constexpr bool can_hold_value(value_type v) noexcept {
            auto value = (underlying_integral_type)v;
            if constexpr (is_signed_value_type) {
               if (value > 0 && value & sign_bit_mask) {
                  return false;
               }
               constexpr const auto sign_mask = ~value_mask;
               if (value < 0 && (value & sign_mask) != sign_mask) {
                  return false;
               }
            } else {
               if (value >= (underlying_integral_type(1) << value_bitcount)) {
                  return false;
               }
            }
            return true;
         }

         constexpr value_type get(size_t i) const {
            _bounds_check(i);
            underlying_integral_type value = 0;
            _access_element(
               i,
               [&value](const unit_type unit) {
                  value = unit;
               },
               [&value](const unit_type unit, uint8_t bitshift, uint8_t bitcount, size_t remaining_value_bits) {
                  value = unit;
                  value = (value >> (unit_bitcount - bitshift - bitcount)) & cobb::bits::all_ones<underlying_integral_type>(bitcount);
               },
               [&value](const unit_type unit, uint8_t trailing_bitcount, size_t whole_unit_bits_remaining) {
                  value <<= unit_bitcount;
                  value |= unit;
               },
               [&value](const unit_type unit, uint8_t trailing_bitcount) {
                  value <<= trailing_bitcount;

                  auto mask = cobb::bits::all_ones<unit_type>(trailing_bitcount);
                  value |= (unit >> (unit_bitcount - trailing_bitcount)) & mask;
               }
            );
            value = _sign_extend_raw_bits(value);
            return (T)value;
         }
         constexpr void set(size_t i, value_type v) {
            _bounds_check(i);
            if (std::is_constant_evaluated()) {
               if (!can_hold_value(v))
                  throw;
            }
            auto value = (underlying_integral_type)v;
            _access_element(
               i,
               [value](unit_type& unit) {
                  unit = value;
               },
               [value](unit_type& unit, uint8_t bitshift, uint8_t bitcount, size_t remaining_value_bits) {
                  uint8_t clear_ls = unit_bitcount - bitshift - bitcount;

                  unit &= ~(cobb::bits::all_ones<unit_type>(bitcount) << clear_ls);
                  if (bitcount == value_bitcount) {
                     unit |= value << clear_ls;
                  } else {
                     uint8_t write_rs = (value_bitcount - (unit_bitcount - bitshift));
                     unit |= value >> write_rs; // should be no need to mask; we're writing the leading bits in the value
                  }
               },
               [value](unit_type& unit, uint8_t trailing_bitcount, size_t whole_unit_bits_remaining) {
                  unit = value >> (trailing_bitcount + whole_unit_bits_remaining);
               },
               [value](unit_type& unit, uint8_t trailing_bitcount) {
                  unit = cobb::bits::replace_most_significant_bits(unit, (unit_type)value, trailing_bitcount);
               }
            );
            return;
         }

         void clear() {
            this->units = {};
         }

         constexpr value_wrapper operator[](size_t i) {
            _bounds_check(i);
            return value_wrapper(*this, i);
         }
         constexpr const_value_wrapper operator[](size_t i) const {
            _bounds_check(i);
            return const_value_wrapper(*this, i);
         }
   };
}