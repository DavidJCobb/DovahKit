#pragma once
#include "./bs_hash.h"
#include <array>
#include <type_traits>

namespace dovah {
   namespace impl {
      // work around constexpr unions not allowing type punning, and hold some helper functions while we're at it
      struct _bs_hash_constexpr {
         bs_hash data;

         static constexpr char to_lower(char c) {
            if (c >= 'A' && c <= 'Z')
               c += 0x20;
            else if (c == '\\') // not from Skyrim; done for our own convenience
               c = '/';
            return c;
         }

         static constexpr uint32_t hash_string(std::string_view view) {
            uint32_t hash = 0;
            for (auto c : view) {
               hash *= 0x1003F;
               hash += to_lower(c);
            }
            return hash;
         }
         
         template<typename T, size_t Index> requires (Index < (8 / sizeof(T)))
         constexpr T get_part() {
            if (std::is_constant_evaluated()) {
               constexpr const size_t bitcount = sizeof(T) * 8;
               constexpr const size_t limit    = 64 / bitcount - 1;

               constexpr const size_t shift_by = (std::endian::native == std::endian::little) ? (Index * bitcount) : ((limit - Index) * bitcount);
               //
               return (T)(data.value >> shift_by);
            } else {
               if constexpr (sizeof(T) == 4)
                  return data.dwords[Index];
               else if constexpr (sizeof(T) == 1)
                  return data.bytes[Index];
            }
         }
         
         template<typename T, size_t Index> requires (Index < (8 / sizeof(T)))
         constexpr void set_part(T v) {
            if (std::is_constant_evaluated()) {
               constexpr const size_t bitcount = sizeof(T) * 8;
               constexpr const size_t limit    = 64 / bitcount - 1;

               constexpr const size_t shift_by = (std::endian::native == std::endian::little) ? (Index * bitcount) : ((limit - Index) * bitcount);

               constexpr const T max = (T)-1;
               
               data.value &= ~((uint64_t)max << shift_by);
               data.value |= (uint64_t)v << shift_by;
            } else {
               if constexpr (sizeof(T) == 4)
                  data.dwords[Index] = v;
               else if constexpr (sizeof(T) == 1)
                  data.bytes[Index] = v;
            }
         }
         
         template<typename T, size_t Index> requires (Index < (8 / sizeof(T)))
         constexpr void add_part(T v) {
            if (std::is_constant_evaluated()) {
               set_part<T, Index>(get_part<T, Index>() + v);
            } else {
               if constexpr (sizeof(T) == 4)
                  data.dwords[Index] += v;
               else if constexpr (sizeof(T) == 1)
                  data.bytes[Index] += v;
            }
         }
      };
   }

   /*static*/ constexpr bs_hash bs_hash::via_skyrim_algorithm(std::string_view path, std::string_view extension) {
      impl::_bs_hash_constexpr out;
      
      auto _to_lower = &impl::_bs_hash_constexpr::to_lower;

      if (path.empty())
         return out.data;

      // Byte 0: to_lower'd last character, or 0
      // Byte 1: to_lower'd penultimate character, or 0
      // Byte 2: length, truncated
      // Byte 3: to_lower'd first character
      out.set_part<uint8_t, 2>(path.size());
      out.set_part<uint8_t, 3>(_to_lower(path[0]));
      if (path.size() > 2) {
         out.set_part<uint8_t, 0>(_to_lower(path[path.size() - 1]));
         out.set_part<uint8_t, 1>(_to_lower(path[path.size() - 2]));

         if (path.size() > 3) {
            path.remove_prefix(1);
            path.remove_suffix(2);
            out.set_part<uint32_t, 1>(impl::_bs_hash_constexpr::hash_string(path));
         }
      }

      if (extension.empty()) // NOTE: there's a difference between nullptr and ""; Bethesda seems to use the former as a sentinel, and is what we should be checking for here
         return out.data;

      out.add_part<uint32_t, 1>(impl::_bs_hash_constexpr::hash_string(extension));

      constexpr const std::array<std::string_view, 6> well_known_extensions = {
         "",
         ".nif",
         ".kf",
         ".dds",
         ".wav",
         ".adp", // not present in Oblivion
      };
      constexpr const size_t longest = [well_known_extensions]() {
         size_t i = 0;
         for (auto& ext : well_known_extensions)
            if (ext.size() > i)
               i = ext.size();
         return i;
      }();
      if (extension.size() <= longest) {
         for (uint8_t i = 0; i < well_known_extensions.size(); ++i) {
            if (extension == well_known_extensions[i]) {
               out.add_part<uint8_t, 3>(uint8_t(i & ~3) << 5);
               out.add_part<uint8_t, 0>(uint8_t(i & ~1) << 6);
               out.add_part<uint8_t, 1>(uint8_t(i) << 7);
            }
         }
      }

      return out.data;
   }
}