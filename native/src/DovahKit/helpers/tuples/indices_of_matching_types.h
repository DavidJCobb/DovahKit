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
#include <tuple>
#include <type_traits>
#include "./filter_types.h"

namespace cobb::tuples {
   namespace impl::_indices_of_matching_types {
      template<typename Tuple, auto Predicate>
      static constexpr size_t matching_count = std::tuple_size_v<filter_types<Tuple, Predicate>>;

      template<typename Tuple, auto Predicate>
      using match_list_t = std::array<size_t, matching_count<Tuple, Predicate>>;

      template<typename Tuple, auto Predicate>
      struct result;

      template<auto Predicate, typename... Types>
      struct result<std::tuple<Types...>, Predicate> {
         static constexpr auto value = [](){
            match_list_t<std::tuple<Types...>, Predicate> out = {};
            size_t i = 0;
            size_t o = 0;
            (
               (
                  Predicate.template operator()<Types>() ?
                     (out[o++] = i++)
                  :
                     i++
               ),
               ...
            );
            return out;
         }();
      };

      template<auto Predicate>
      struct result<std::tuple<>, Predicate> {
         static constexpr match_list_t<std::tuple<>, Predicate> value = {};
      };
   }

   template<typename Tuple, auto Predicate>
   constexpr impl::_indices_of_matching_types::match_list_t<Tuple, Predicate> indices_of_matching_types
      = impl::_indices_of_matching_types::result<Tuple, Predicate>::value;
}