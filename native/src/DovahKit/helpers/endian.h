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
#include <bit>
#include <cstdint>
#include <cstdlib>
#include "function_traits.h"

namespace cobb {
   namespace impl::endian {
      template<auto F> concept TransformSingleArgumentFunction = requires {
         typename function_traits<decltype(F)>;
         requires function_traits<decltype(F)>::arg_count == 1;
         requires std::is_same_v<member_function_context_type<F>, void>;
         requires std::is_same_v<type_of_nth_argument<F, 0>, return_type_of<F>>;
      };

      template<typename T, int C = sizeof(T)> struct raw_byteswap_intrinsic;
      template<typename T> struct raw_byteswap_intrinsic<T, 8> {
         static constexpr auto function = &_byteswap_uint64;
      };
      template<typename T> struct raw_byteswap_intrinsic<T, 4> {
         static constexpr auto function = &_byteswap_ulong;
      };
      template<typename T> struct raw_byteswap_intrinsic<T, 2> {
         static constexpr auto function = &_byteswap_ushort;
      };
   }

   template<typename T> concept HasByteswapIntrinsic = requires {
      typename impl::endian::raw_byteswap_intrinsic<T>;
      requires impl::endian::TransformSingleArgumentFunction<impl::endian::raw_byteswap_intrinsic<T>::function>;
      requires sizeof(T) == sizeof(return_type_of<impl::endian::raw_byteswap_intrinsic<T>::function>);
   };

   // Describes an intrinsic byteswap function.
   template<typename T> requires HasByteswapIntrinsic<T>
   struct byteswap_intrinsic {
      static constexpr auto function = impl::endian::raw_byteswap_intrinsic<T>::function;
      using integral_type = return_type_of<function>;

      static_assert(sizeof(T) == sizeof(integral_type));
      
      static T execute(T value) {
         return std::bit_cast<T, integral_type>( function( std::bit_cast<integral_type, T>(value) ) );
      }
   };

   template<typename T> requires HasByteswapIntrinsic<T>
   [[nodiscard]] constexpr T intrinsic_byteswap(T value) {
      using bi = byteswap_intrinsic<T>;
      using it = bi::integral_type;
      return std::bit_cast<T, it>( bi::function( std::bit_cast<it, T>(value) ) );
   }

   template<typename T> [[nodiscard]] constexpr T byteswap(T value) {
      if constexpr (sizeof(T) == 1) {
         return value;
      }
      if (!std::is_constant_evaluated()) { // the byteswap intrinsics aren't constexpr
         if constexpr (HasByteswapIntrinsic<T>) {
            return intrinsic_byteswap(value);
         }
      }
      std::is_array<uint8_t, sizeof(T)> bytes;
      const uint8_t* raw = (const uint8_t*)&value;
      for (size_t i = 0; i < bytes.size(); ++i) {
         bytes[i] = raw[bytes.size() - i - 1];
      }
      return std::bit_cast<T, decltype(bytes)>(bytes);
   }

   template<std::endian which, typename T> [[nodiscard]] constexpr T endian_cast(T value) {
      if constexpr (std::endian::native == which)
         return value;
      return byteswap<T>(value);
   }
}