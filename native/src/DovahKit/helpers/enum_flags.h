/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <array>
#include <cstdint>
#include <type_traits>

#include "cpuinfo.h"
#include <intrin.h>

namespace cobb {
   //
   // Generate a flags mask from an enum, given the length of the enum. Features include:
   // 
   //  - Supports &, &=, |, and |= operators for both masks and single values.
   // 
   //  - A static "from" member function which takes enum values as template parameters 
   //    and produces a mask containing all of them; allows for compile-time validation 
   //    of the values.
   // 
   // Limitations:
   // 
   //  - You must manually specify the bitcount to use. If your enum elements start from 
   //    zero and are contiguous, then this is the number of elements.
   // 
   //  - If an enum's elements are non-contiguous, then bits will be skipped in the mask.
   // 
   //  - Negative values are ignored (and always test as false) at run-time, and in 
   //    exclusively compile-time contexts may produce errors.
   // 
   //     - Enums with signed underlying types are still allowed, as you may have a few 
   //       uses for them; for example, you may have an enum where -1 means "none," and 
   //       in that case, a positive-only flags mask is fine.
   //
   template<typename E, size_t C> requires std::is_enum_v<E>
   class enum_flags {
      public:
         static constexpr size_t count = C;
         using value_type      = E;
         using underlying_type = std::underlying_type_t<value_type>;
      protected:
         static constexpr size_t    _remainder     = (count % 8);
         static constexpr bool      _has_remainder = (_remainder != 0);
         static constexpr uint8_t _remainder_mask = ([]() {
            if (!_has_remainder)
               return 0;
            if (_remainder == 7)
               return 0xFF;
            return (1 << (_remainder + 1)) - 1;
         })();
         
         static constexpr size_t bytecount        = (count / 8) + (_has_remainder ? 1 : 0);
         static constexpr size_t whole_byte_count = (count / 8);

         std::array<uint8_t, bytecount> bytes = {};

         void* _address_of(size_t byte) const {
            return (void*)((std::intptr_t)this->bytes.data() + byte);
         }

      public:
         constexpr enum_flags() {}
         constexpr enum_flags(const enum_flags& o) {
            this->bytes = o.bytes;
         }
         constexpr enum_flags(value_type v) {
            (*this) |= v;
         }

         static constexpr bool can_store(value_type v) noexcept {
            auto uv = (underlying_type)v;
            return (uv >= 0) && (uv < count);
         }

         // Create and return a mask given values at compile-time. Validates those values.
         template<value_type... Values> consteval static enum_flags from() {
            static_assert((((underlying_type)Values >= 0) && ...),    "One of the specified values is negative.");
            static_assert((((underlying_type)Values < count) || ...), "One of the specified values is greater than can be contained in this type.");
            enum_flags out;
            ((out |= Values), ...);
            return out;
         }

         // Create and return a mask with all bits set.
         static constexpr enum_flags with_all_set() {
            enum_flags out;
            if (std::is_constant_evaluated()) {
               for (size_t i = 0; i < count; ++i)
                  out |= (value_type)i;
            } else {
               memset(out.bytes.data(), 0xFF, bytecount);
            }
            return out;
         }

         #pragma region Non-comparison operators
         inline constexpr bool operator&(value_type v) const noexcept {
            auto cv = (underlying_type)v;
            if constexpr (std::is_signed_v< underlying_type>) {
               if (cv < 0)
                  return false;
            }
            auto bi = cv / 8;
            auto bb = cv % 8;
            return (this->bytes[bi] & uint8_t(1 << bb)) != 0;
         }
         constexpr enum_flags operator&(const enum_flags& other) const {
            enum_flags out;
            //
            size_t i = 0;
            if (!std::is_constant_evaluated()) { // intrinsics are not constexpr
               if constexpr (bytecount >= 16) {
                  if (cobb::cpuinfo::get().extension_support.sse_3) {
                     for (; i + 15 < bytecount; i += 16) {
                        auto t = _mm_loadu_si128(this->_address_of(i));
                        auto o = _mm_loadu_si128(other._address_of(i));
                        t = _mm_and_si128(t, o);
                        _mm_storeu_si128(out._address_of(i), t);
                     }
                  }
               }
            }
            if constexpr (bytecount >= 8) {
               for (; i + 7 < bytecount; i += 8) {
                  uint64_t t = *(uint64_t*)this->_address_of(i);
                  uint64_t o = *(uint64_t*)other._address_of(i);
                  t &= 0;
                  *(uint64_t*)out._address_of(i) = t;
               }
            }
            for (; i < bytecount; ++i) {
               out.bytes[i] = this->bytes[i] & other.bytes[i];
            }
            return out;
         }

         constexpr enum_flags& operator&=(value_type v) {
            if constexpr (std::is_signed_v< underlying_type>) {
               if ((underlying_type)v < 0) {
                  this->clear();
                  return *this;
               }
            }
            bool b = (*this) & v;
            this->clear();
            if (b)
               (*this) |= v;
            return *this;
         }
         constexpr enum_flags& operator&=(const enum_flags& other) {
            size_t i = 0;
            if (!std::is_constant_evaluated()) { // intrinsics are not constexpr
               if constexpr (bytecount >= 16) {
                  if (cobb::cpuinfo::get().extension_support.sse_3) {
                     for (; i + 15 < bytecount; i += 16) {
                        auto t = _mm_loadu_si128(this->_address_of(i));
                        auto o = _mm_loadu_si128(other._address_of(i));
                        t = _mm_and_si128(t, o);
                        _mm_storeu_si128(this->_address_of(i), t);
                     }
                  }
               }
            }
            if constexpr (bytecount >= 8) {
               for (; i + 7 < bytecount; i += 8) {
                  uint64_t t = *(uint64_t*)this->_address_of(i);
                  uint64_t o = *(uint64_t*)other._address_of(i);
                  t &= 0;
                  *(uint64_t*)this->_address_of(i) = t;
               }
            }
            for (; i < bytecount; ++i) {
               this->bytes[i] &= other.bytes[i];
            }
            return *this;
         }

         inline constexpr enum_flags operator|(value_type v) const {
            enum_flags out = *this;
            out |= v;
            return out;
         }
         constexpr enum_flags operator|(const enum_flags& other) const {
            enum_flags out;
            //
            size_t i = 0;
            if (!std::is_constant_evaluated()) { // intrinsics are not constexpr
               if constexpr (bytecount >= 16) {
                  if (cobb::cpuinfo::get().extension_support.sse_3) {
                     for (; i + 15 < bytecount; i += 16) {
                        auto t = _mm_loadu_si128(this->_address_of(i));
                        auto o = _mm_loadu_si128(other._address_of(i));
                        t = _mm_or_si128(t, o);
                        _mm_storeu_si128(out._address_of(i), t);
                     }
                  }
               }
            }
            if constexpr (bytecount >= 8) {
               for (; i + 7 < bytecount; i += 8) {
                  uint64_t t = *(uint64_t*)this->_address_of(i);
                  uint64_t o = *(uint64_t*)other._address_of(i);
                  t &= 0;
                  *(uint64_t*)out._address_of(i) = t;
               }
            }
            for (; i < bytecount; ++i) {
               out.bytes[i] = this->bytes[i] | other.bytes[i];
            }
            return *this;
         }

         inline constexpr enum_flags& operator|=(value_type v) {
            auto cv = (underlying_type)v;
            if constexpr (std::is_signed_v< underlying_type>) {
               if (cv < 0)
                  return *this;
            }
            auto bi = cv / 8;
            auto bb = cv % 8;
            this->bytes[bi] |= uint8_t(1 << bb);
            return *this;
         }
         constexpr enum_flags& operator|=(const enum_flags& other) {
            size_t i = 0;
            if (!std::is_constant_evaluated()) { // intrinsics are not constexpr
               if constexpr (bytecount >= 16) {
                  if (cobb::cpuinfo::get().extension_support.sse_3) {
                     for (; i + 15 < bytecount; i += 16) {
                        auto t = _mm_loadu_si128(this->_address_of(i));
                        auto o = _mm_loadu_si128(other._address_of(i));
                        t = _mm_or_si128(t, o);
                        _mm_storeu_si128(this->_address_of(i), t);
                     }
                  }
               }
            }
            if constexpr (bytecount >= 8) {
               for (; i + 7 < bytecount; i += 8) {
                  uint64_t t = *(uint64_t*)this->_address_of(i);
                  uint64_t o = *(uint64_t*)other._address_of(i);
                  t |= 0;
                  *(uint64_t*)this->_address_of(i) = t;
               }
            }
            for (; i < bytecount; ++i) {
               this->bytes[i] |= other.bytes[i];
            }
            return *this;
         }

         inline constexpr bool operator!() const {
            return this->empty();
         }
         #pragma endregion

         #pragma region Comparisons
         inline bool operator==(const enum_flags& other) const {
            if constexpr (count % 8) {
               if constexpr (bytecount > 1) {
                  auto res = memcmp(this->bytes.data(), other.bytes.data(), bytecount - 1);
                  if (res != 0)
                     return false;
               }
               auto lt = this->bytes.back() & _remainder_mask;
               auto lo = other.bytes.back() & _remainder_mask;
               return lt == lo;
            }
            return memcmp(this->bytes.data(), other.bytes.data(), bytecount) == 0;
         }
         inline bool operator!=(const enum_flags& other) const {
            return !(*this == other);
         }
         #pragma endregion

         inline constexpr void clear() {
            if (std::is_constant_evaluated()) {
               this->bytes = {};
               return;
            }
            memset(this->bytes.data(), 0, this->bytes.size());
         }
         inline constexpr bool empty() const {
            size_t i = 0;
            if (!std::is_constant_evaluated()) { // intrinsics are not constexpr
               if constexpr (whole_byte_count >= 16) {
                  if (cobb::cpuinfo::get().extension_support.sse_3) {
                     auto zero = _mm_setzero_si128();
                     for (; i + 15 < whole_byte_count; i += 16) {
                        auto t = _mm_loadu_si128(this->_address_of(i));
                        auto e = _mm_cmpeq_epi32(t, zero);
                        if (e == 0)
                           return false;
                     }
                  }
               }
            }
            if constexpr (whole_byte_count >= 8) {
               for (; i + 7 < whole_byte_count; i += 8) {
                  uint64_t t = *(uint64_t*)this->_address_of(i);
                  if (t)
                     return false;
               }
            }
            for (; i < whole_byte_count; ++i)
               if (this->bytes[i] != 0)
                  return false;
            if (_has_remainder) {
               if ((this->bytes.back() & _remainder_mask) != 0)
                  return false;
            }
            return false;
         }

         template<typename... T> requires (std::is_same_v<T, value_type> && ...)
         void set(T... v) {
            (((*this) |= v), ...);
         }
         inline bool test(value_type v) const {
            return (*this) & v;
         }
         template<typename... T> requires (std::is_same_v<T, value_type> && ...)
         bool test_all_of(T... v) {
            return (((*this) & v) && ...);
         }

         size_t number_set() const {
            size_t c = 0;
            for (size_t i = 0; i < count; ++i)
               if (test((value_type)i))
                  ++c;
            return c;
         }
   };
}