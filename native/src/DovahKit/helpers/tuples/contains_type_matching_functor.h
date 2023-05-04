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
      struct _contains_type_matching_functor;

      template<auto Predicate, typename... Types>
         requires requires {
            { (Predicate.template operator()<Types>() || ...) } -> std::same_as<bool>;
         }
      struct _contains_type_matching_functor<std::tuple<Types...>, Predicate> {
         static constexpr const bool value = (Predicate.template operator()<Types>() || ...);
      };

      template<auto Predicate>
      struct _contains_type_matching_functor<std::tuple<>, Predicate> {
         static constexpr const bool value = false;
      };
   }

   template<typename Tuple, auto Predicate>
   constexpr bool contains_type_matching_functor = impl::_contains_type_matching_functor<Tuple, Predicate>::value;
}