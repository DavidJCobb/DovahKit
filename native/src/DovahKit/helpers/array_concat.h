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
   namespace impl::array_concat {
      template<typename... Types> struct total_size {
         static constexpr size_t value = (std::tuple_size_v<Types> +...);
      };
      template<typename... Types> struct value_type {
         template<typename T, typename... Types> struct first {
            using type = T::value_type;
         };

         using type = first<Types...>::type;
      };
   }

   template<typename... Types> requires requires(Types... args) {
      requires (is_std_array<Types> && ...);
      requires (std::is_same_v<typename impl::array_concat::value_type<Types...>::type, typename Types::value_type> && ...);
   }
   constexpr std::array<
      typename impl::array_concat::value_type<Types...>::type,
      impl::array_concat::total_size<Types...>::value
   > array_concat(Types... arrays) {
      using value_type = typename impl::array_concat::value_type<Types...>::type;
      constexpr size_t size = impl::array_concat::total_size<Types...>::value;
      //
      std::array<value_type, size> out = {};
      size_t i = 0;
      (
         (
            std::copy(arrays.cbegin(), arrays.cend(), out.begin() + i),
            i += arrays.size()
         ),
         ...
      );
      return out;
   }
}