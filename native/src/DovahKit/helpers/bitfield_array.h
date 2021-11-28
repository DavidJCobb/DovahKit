#pragma once
#include <cstdint>
#include <string>

namespace cobb {
   template<typename T, size_t count, size_t bits> class bitfield_array {
      protected:
         using value_type = T;
         using size_type  = size_t;
         static constexpr auto bitcount_per_item = bits;
      protected:
         static constexpr size_t total_bitcount  = count * bitcount_per_item;
         static constexpr size_t total_bytecount = (total_bitcount / 8) + ((total_bitcount % 8) ? 1 : 0);

         using unit_type = uint8_t;
         static constexpr size_t    bits_per_unit = sizeof(unit_type) * 8;
         static constexpr unit_type item_mask     = ((1 << bitcount_per_item) - 1);

         unit_type bytes[total_bytecount];

         struct value_wrapper {
            bitfield_array& target;
            size_t index;

            value_wrapper(bitfield_array& ba, size_t i) : target(ba), index(i) {}

            inline value_wrapper& operator=(const T& v) {
               target.set(index, v);
               return *this;
            }
            inline operator T() const {
               return target.get(index);
            }
         };
         struct const_value_wrapper {
            const bitfield_array& target;
            size_t index;

            const_value_wrapper(const bitfield_array& ba, size_t i) : target(ba), index(i) {}

            inline operator T() const {
               return target.get(index);
            }
         };

      public:
         bitfield_array() {
            memset(&bytes, 0, sizeof(bytes));
         }

         T get(size_t i) const {
            size_t  bit   = i * bitcount_per_item;
            size_t  byte  = bit / bits_per_unit;
            uint8_t shift = bit % bits_per_unit;
            //
            auto value = (bytes[byte] >> shift) & item_mask;
            return (T)value;
         }
         void set(size_t i, T v) {
            size_t  bit   = i * bitcount_per_item;
            size_t  byte  = bit / bits_per_unit;
            uint8_t shift = bit % bits_per_unit;
            //
            auto& value = bytes[byte];
            value &= ~(item_mask << shift);
            value |= ((uint8_t)v) << shift;
         }

         bool is_nonzero(size_t i) const {
            size_t  bit   = i * bitcount_per_item;
            size_t  byte  = bit / bits_per_unit;
            uint8_t shift = bit % bits_per_unit;
            //
            auto value = (bytes[byte] >> shift);
            return (value & item_mask) != 0;
         }

         void clear() {
            memset(&bytes, 0, sizeof(bytes));
         }

         value_wrapper operator[](int i) {
            return value_wrapper(*this, i);
         }
         const_value_wrapper operator[](int i) const {
            return const_value_wrapper(*this, i);
         }
   };
}