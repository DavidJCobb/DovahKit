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
#include <concepts>
#include <tuple>
#include "tuples/contains_type.h"
#include "tuples/contains_type_matching_functor.h"
#include "tuples/filter_types.h"
#include "tuples/first_index_of_type.h"
#include "tuples/index_of_first_matching_type.h"
#include "tuples/map_types.h"
#include "tuples/unpack_types_into.h"

namespace cobb {
   template<typename... Types> class class_array;

   namespace impl::_class_array {
      template<auto Functor, typename... Types> concept functor_returns_bool = requires {
         { (Functor.template operator()<Types>() || ...) } -> std::same_as<bool>;
      };
   }

   template<typename... Types> class class_array;
   template<typename... Types> class class_array {
      public:
         static constexpr const std::size_t index_of_none = (std::size_t)-1;

         class_array() = delete;
         class_array(const class_array&) = delete;
         class_array(class_array&&) = delete;
         ~class_array() = delete;

      public:
         using as_tuple = std::tuple<Types...>;

         template<std::size_t N> requires (N < sizeof...(Types))
         using nth_type = typename std::tuple_element<N, as_tuple>::type;

         static constexpr size_t count = sizeof...(Types);
         static consteval size_t size() noexcept { return count; }

         template<typename T>
         static constexpr bool contains_type = cobb::tuples::contains_type<as_tuple, T>;
         template<typename T>
         static constexpr std::size_t index_of_type = cobb::tuples::first_index_of_type<as_tuple, T>;

         template<auto Functor>
         static constexpr bool contains_matching_type = cobb::tuples::contains_type_matching_functor<as_tuple, Functor>;
         template<auto Functor>
         static constexpr std::size_t index_of_matching_type = cobb::tuples::index_of_first_matching_type<as_tuple, Functor>;
         template<auto Functor>
         using get_matching_type = nth_type<index_of_matching_type<Functor>>;

         template<auto Functor, typename... Args>
         static constexpr void for_each(Args&&... args) {
            (Functor.template operator()<Types>(std::forward<Args>(args)...), ...);
         }

         template<typename Functor, typename... Args>
         static constexpr void for_each(Functor&& f, Args&&... args) {
            (f.template operator()<Types>(std::forward<Args>(args)...), ...);
         }
         
         template<auto Functor, typename... Args>
         static constexpr bool for_each_until_false(Args&&... args) {
            return (Functor.template operator()<Types>(std::forward<Args>(args)...) && ...);
         }
         template<typename Functor, typename... Args>
         static constexpr bool for_each_until_false(Functor&& f, Args&&... args) {
            return (f.template operator()<Types>(std::forward<Args>(args)...) && ...);
         }
         
         template<auto Functor, typename... Args>
         static constexpr bool for_each_until_true(Args&&... args) {
            return (Functor.template operator()<Types>(std::forward<Args>(args)...) || ...);
         }
         template<typename Functor, typename... Args>
         static constexpr bool for_each_until_true(Functor&& f, Args&&... args) {
            return (f.template operator()<Types>(std::forward<Args>(args)...) || ...);
         }
         
         template<typename Result, auto Functor>
            #ifndef __INTELLISENSE__
            requires requires(Result prev) {
               { Functor.template operator()<nth_type<0>>(prev) } -> std::convertible_to<Result>;
            }
            #endif
         static constexpr Result reduce(Result prev = {}) {
            return ((prev = Functor.template operator()<Types>(prev)), ...);
         }

         template<auto Functor>
         using filter_types = cobb::tuples::unpack_types_into<cobb::tuples::filter_types<as_tuple, Functor>, ::cobb::class_array>;
         template<template<typename> typename Transform>
         using map_types = cobb::tuples::unpack_types_into<cobb::tuples::map_types<Transform, as_tuple>, ::cobb::class_array>;
   };
}