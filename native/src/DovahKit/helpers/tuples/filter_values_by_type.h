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
#include <tuple>
#include <type_traits>
#include <utility>
#include "./filter_types.h"
#include "./indices_of_matching_types.h"

namespace cobb::tuples {
   template<auto Predicate, typename Tuple>
   constexpr filter_types<std::decay_t<Tuple>, Predicate> filter_values_by_type(const Tuple& tuple) {
      using result_t = filter_types<std::decay_t<Tuple>, Predicate>;

      constexpr auto index_map = indices_of_matching_types<std::decay_t<Tuple>, Predicate>;
      result_t out = {};
      [&out]<std::size_t... I>(const Tuple& tuple, std::index_sequence<I...>) {
         (
            (std::get<I>(out) = std::get<index_map[I]>(tuple)),
            ...
         );
      }(
         tuple,
         std::make_index_sequence<std::tuple_size_v<std::decay_t<decltype(index_map)>>>{}
      );
      return out;
   }
}