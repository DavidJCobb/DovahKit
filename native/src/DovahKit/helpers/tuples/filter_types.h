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
#include "./concat.h"
#include "./prepend.h"

namespace cobb::tuples {
   namespace impl::_filter_types {
      template<auto Predicate, typename...> struct variadic_filter;
      template<auto Predicate, typename... Types> using variadic_filter_t = typename variadic_filter<Predicate, Types...>::type;
      
      template<auto Predicate> struct variadic_filter<Predicate> {
         using type = std::tuple<>;
      };
      
      template<auto Predicate, typename First, typename... Next>
      struct variadic_filter<Predicate, First, Next...> {
         using type = std::conditional_t<
            (Predicate.template operator()<First>()),
            prepend<First, variadic_filter_t<Predicate, Next...>>,
            variadic_filter_t<Predicate, Next...>
         >;
      };

      template<bool> struct retain {
         template<typename Item> using type = std::tuple<Item>;
      };
      template<> struct retain<false> {
         template<typename Item> using type = std::tuple<>;
      };

      // ---

      template<auto Predicate, typename> struct result;
      template<auto Predicate, typename... Types> struct result<Predicate, std::tuple<Types...>> {
         using type = decltype(
            std::tuple_cat(
               std::declval<
                  typename retain<
                     Predicate.template operator()<Types>()
                  >::template type<Types>
               >()...
            )
         );
      };
   }

   template<typename Tuple, auto Predicate> using filter_types = typename impl::_filter_types::result<Predicate, Tuple>::type;
}