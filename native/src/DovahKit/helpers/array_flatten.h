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
#include <type_traits>
#include "concepts.h"
#include "type_traits/is_std_array.h"

namespace cobb {
   namespace impl::array_flatten {
      template<typename A> concept is_flat_array = requires {
         requires is_std_array<A>;
         typename A::value_type;
         requires !is_std_array<typename A::value_type>;
      };
      template<typename A> concept is_nested_array = requires {
         requires is_std_array<A>;
         typename A::value_type;
         requires is_std_array<typename A::value_type>;
      };

      template<typename T> struct innermost_value_type;
      template<typename T> requires is_flat_array<T> struct innermost_value_type<T> {
         using type = T::value_type;
      };
      template<typename A> requires is_nested_array<A> struct innermost_value_type<A> {
         using type = innermost_value_type<typename A::value_type>::type;
      };

      template<typename T> struct innermost_value_count {
         static constexpr size_t value = 0;
      };
      template<typename T> requires is_flat_array<T> struct innermost_value_count<T> {
         static constexpr size_t value = std::tuple_size_v<T>;
      };
      template<typename T> requires is_nested_array<T> struct innermost_value_count<T> {
         static constexpr size_t value = std::tuple_size_v<T> * innermost_value_count<typename T::value_type>::value;
      };

      template<typename... Types> struct first_innermost_value_type {
         template<typename T, typename... Types> struct first {
            using type = innermost_value_type<T>::type;
         };

         using type = first<Types...>::type;
      };
      template<typename... Types> struct total_innermost_value_count {
         static constexpr size_t value = (innermost_value_count<Types>::value + ...);
      };

      template<typename Dst, typename Src> requires (is_flat_array<Src> || is_nested_array<Src>)
      constexpr size_t flatten(Dst& dst, const Src& src, size_t to) {
         if constexpr (is_flat_array<Src>) {
            std::copy(src.cbegin(), src.cend(), dst.begin() + to);
         } else if constexpr (is_nested_array<Src>) {
            size_t at = to;
            for (const auto& item : src)
               at = flatten(dst, item, at);
         }
         return to + innermost_value_count<Src>::value;
      }
   }

   template<typename... Types> requires requires(Types... args) {
      requires (is_std_array<Types> && ...);
      requires (all_same<typename impl::array_flatten::first_innermost_value_type<Types...>::type, typename impl::array_flatten::innermost_value_type<Types>::type...>);
   }
   constexpr std::array<
      typename impl::array_flatten::first_innermost_value_type<Types...>::type,
      impl::array_flatten::total_innermost_value_count<Types...>::value
   > array_flatten(Types... arrays) {
      using value_type = typename impl::array_flatten::first_innermost_value_type<Types...>::type;
      constexpr size_t size = impl::array_flatten::total_innermost_value_count<Types...>::value;
      //
      std::array<value_type, size> out = {};
      size_t i = 0;
      (
         (i += impl::array_flatten::flatten(out, arrays, i)), ...
      );
      return out;
   }
}