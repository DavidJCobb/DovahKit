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

namespace cobb::tuples {
   namespace impl {
      template<typename Tuple, auto Predicate>
      struct _index_of_first_matching_type;

      template<auto Predicate, typename... Types> requires requires {
         { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
      }
      struct _index_of_first_matching_type<std::tuple<Types...>, Predicate> {
         static constexpr auto value = []() consteval {
            std::size_t i = 0;
            ((Predicate.template operator()<Types>() || (++i, false)) || ...);
            return i < sizeof...(Types) ? i : (std::size_t)-1;
         }();
      };

      template<auto Predicate>
      struct _index_of_first_matching_type<std::tuple<>, Predicate> {
         static constexpr std::size_t value = (std::size_t)-1;
      };
   }

   template<typename Tuple, auto Predicate>
   constexpr std::size_t index_of_first_matching_type = impl::_index_of_first_matching_type<Tuple, Predicate>::value;
}