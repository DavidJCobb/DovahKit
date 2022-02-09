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
#include "concepts.h"
#include "copy_variadic_template_parameters.h"
#include "tuple_concat.h"
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>

namespace cobb {
   namespace class_list_concepts {
      template<template<typename T> typename functor> concept is_for_each_functor = requires {
         { functor<void>::execute() };
      };
      template<template<typename T> typename functor> concept is_for_each_functor_with_bool_result = requires {
         { functor<void>::execute() } -> std::same_as<bool>;
      };
      template<template<typename T> typename functor, typename... Args> concept is_for_each_functor_with_varargs = requires {
         requires true;
         //{ functor<void>::execute(Args{}...) }; // can't be bothered to figure out why this isn't working or whether it's another MSVC2019 bug right now...
      };

      //
      // We take the lambda's type and the list of types for a class list, so that we can ensure that the lambda 
      // accepts all of those types (i.e. if it uses constraints, all types in the list meet those constraints). 
      // The main thing we want to test is whether the lambda is templated on a single type; however, there's no 
      // way to retrieve that information directly, and just testing against void or whatever would fail when it 
      // uses constraints.
      //
      template<typename functor, typename... Types> concept is_for_each_lambda = requires (functor&& f) {
         { (f.template operator()<Types>(), ...) };
      };
      template<typename functor, typename... Types> concept is_for_each_lambda_with_bool_result = requires (functor&& f) {
         { (f.template operator()<Types>() || ...) } -> std::same_as<bool>;
      };
   }

   //
   // Template for defining a list of classes, and being able to loop over them with 
   // a functor. Usage example:
   // 
   //    struct foo { static constexpr int value = 5; };
   //    struct bar { static constexpr int value = 7; };
   //    
   //    using my_classes = cobb::class_list<foo, bar>;
   //    
   //    template<typename T> struct functor {
   //       void execute() {
   //          std::cout << T::value << '\n';
   //       }
   //    };
   //    
   //    int main() {
   //       my_classes::for_each<functor>();
   //       //
   //       return 0;
   //    }
   // 
   // Other options include:
   // 
   //    my_classes::for_each_breakable<functor>(); // functor<T>::execute returns bool; stop early if true
   // 
   //    my_classes::for_each_with_args<functor>(1, 2, 3); // calls functor<T>::execute(1, 2, 3)
   // 
   //    my_classes::for_each_breakable_with_args<functor>(1, 2, 3);
   // 
   // Sadly, the functor must be wrapped in a struct for this to work, as C++ only 
   // allows template template parameters to be used for types.
   // 
   // UPDATE: Lambdas are now usable, though MSVC bugs limit their utility in some cases.
   //
   template<typename... Types> class class_list {
      protected:
         template<template<typename> typename T> static constexpr bool is_for_each_functor                  = class_list_concepts::is_for_each_functor<T>;
         template<template<typename> typename T> static constexpr bool is_for_each_functor_with_bool_result = class_list_concepts::is_for_each_functor_with_bool_result<T>;
         template<template<typename> typename T, typename... Args> static constexpr bool is_for_each_functor_with_varargs = class_list_concepts::is_for_each_functor_with_varargs<T, Args...>;
         //
         template<typename T> static constexpr bool is_for_each_lambda                  = class_list_concepts::is_for_each_lambda<T, Types...>;
         template<typename T> static constexpr bool is_for_each_lambda_with_bool_result = class_list_concepts::is_for_each_lambda_with_bool_result<T, Types...>;

      public:
         static constexpr size_t count = sizeof...(Types);
         static constexpr size_t size() noexcept { return count; }

         using as_tuple = std::tuple<Types...>;
         template<size_t n> using nth_type = typename std::tuple_element<n, as_tuple>::type;

         template<typename T> static constexpr bool contains = (std::is_same_v<T, Types> || ...);

         template<typename T> static consteval size_t index_of() {
            size_t index = 0;
            (
               (std::is_same_v<T, Types> ?
                  false             // if match: set result to current index; use false to short-circuit the "and" operator and stop iteration
                : ((++index), true) // no match: increment current index; use true to avoid short-circuiting, and continue iteration
               )
               && ...
            );
            return index < count ? index : size_t(-1);
         };

         #pragma region concat
      protected:
         template<typename> struct _concat_helper;
         template<typename... Others> struct _concat_helper<cobb::class_list<Others...>> {
            using type = cobb::class_list<Types..., Others...>;
         };
         template<typename... Others> struct _concat_helper<std::tuple<Others...>> {
            using type = cobb::class_list<Types..., Others...>;
         };
      public:
         // Concatenate the type lists of two cobb::class_lists, or of a cobb::class_list and a std::tuple.
         template<typename Other> using concat = _concat_helper<Other>::type;
         #pragma endregion

         template<template<typename T> typename Transform> using transform = cobb::class_list<typename Transform<Types>::type...>;

      public:
         #pragma region Lambda functions
         template<typename lambda> requires is_for_each_lambda<lambda>
         static constexpr void for_each(lambda&& f) {
            (f.template operator()<Types>(), ...);
         }

         template<typename lambda> requires is_for_each_lambda_with_bool_result<lambda>
         static constexpr bool for_each_breakable(lambda&& f) {
            return (... || f.template operator()<Types>());
         }
         
         template<typename lambda> requires is_for_each_lambda_with_bool_result<lambda>
         static constexpr size_t index_of_matching(lambda&& f) {
            size_t index = 0;
            (
               (f.template operator()<Types>() ?
                  false             // if match: set result to current index; use false to short-circuit the "and" operator and stop iteration
                : ((++index), true) // no match: increment current index; use true to avoid short-circuiting, and continue iteration
               )
               && ...
            );
            return index < count ? index : size_t(-1);
         }

         template<typename lambda> requires is_for_each_lambda_with_bool_result<lambda>
         static constexpr bool has_matching(lambda&& f) {
            return (f.template operator()<Types>() || ...);
         }

         #if !defined(_MSC_VER) || _MSC_VER >= 1930
         //
         // MSVC2019 bug 1498687: templated lambdas do not work with templated using declarations; fixed in MSVC2022
         // MSVC2019 bug ???????: attempting to work around the above will trigger an internal compiler error (C1001, impossible to debug)
         // MSVC2019 bug ???????: requirements, and distinctions between type and non-type template parameters, are not needed; this overload conflicts with the functor version
         //
         template<auto lambda> requires is_for_each_lambda_with_bool_result<decltype(lambda)>
         using get_matching = nth_type<index_of_matching(lambda)>;
         #endif

      protected:
         template<typename T, auto lambda> requires is_for_each_lambda_with_bool_result<decltype(lambda)>
         using _matching_type_or_empty_tuple = std::conditional_t<lambda.template operator()<T>(), std::tuple<T>, std::tuple<>>;
      public:
         template<auto lambda> requires is_for_each_lambda_with_bool_result<decltype(lambda)>
         using all_matching = cobb::copy_variadic_template_parameters<
            cobb::class_list,
            cobb::tuple_concat<_matching_type_or_empty_tuple<Types, lambda>...>
         >;
         #pragma endregion

         #pragma region Functor-struct functions
         template<template<typename T> typename functor> requires is_for_each_functor_with_bool_result<functor>
         static constexpr size_t index_of_matching() {
            size_t index = 0;
            (
               (functor<Types>::execute() ?
                  false             // if match: set result to current index; use false to short-circuit the "and" operator and stop iteration
                : ((++index), true) // no match: increment current index; use true to avoid short-circuiting, and continue iteration
               )
               && ...
            );
            return index < count ? index : size_t(-1);
         }

         template<template<typename T> typename functor> requires is_for_each_functor_with_bool_result<functor>
         static constexpr bool has_matching() {
            return (functor<Types>::execute() || ...);
         }

         template<template<typename T> typename functor> requires is_for_each_functor_with_bool_result<functor>
         using get_matching = nth_type<index_of_matching<functor>()>;

         template<template<typename T> typename functor> requires is_for_each_functor<functor>
         static constexpr void for_each() {
            (functor<Types>::execute(), ...);
         }

         template<template<typename T> typename functor> requires is_for_each_functor_with_bool_result<functor>
         static constexpr bool for_each_breakable() {
            return (functor<Types>::execute() || ...);
         }

         template<template<typename T> typename functor, typename... Args> requires is_for_each_functor_with_varargs<functor, Args...>
         static constexpr void for_each_with_args(Args&&... a) {
            (functor<Types>::execute(std::forward<Args>(a)...), ...);
         }
         
         template<template<typename T> typename functor, typename... Args>
         static constexpr bool for_each_breakable_with_args(Args&&... a) {
            return ((functor<Types>::execute(std::forward<Args>(a)...)) || ...);
         }

         template<template<typename T> typename functor, typename U>
         static constexpr auto for_each_with_previous(U prev = U()) -> U {
            return ((prev = functor<Types>::execute(prev)), ...);
         }
         #pragma endregion
   };

   template<typename Tuple> using class_list_from_tuple = cobb::copy_variadic_template_parameters<cobb::class_list, Tuple>;

   // ---

   namespace class_list_with_data_concepts {
      template<template<typename T> typename functor, typename data_type> concept is_for_each_functor = requires (data_type v) {
         { functor<void>::execute(v) };
      };
      template<template<typename T> typename functor, typename data_type> concept is_for_each_functor_with_bool_result = requires (data_type v) {
         { functor<void>::execute(v) } -> std::same_as<bool>;
      };
   }
   namespace impl::class_list_with_data {
      extern constexpr auto dummy_array = std::array{ 0 }; // hopefully prevent MSVC and IntelliSense from complaining too much
   }

   //
   // A variant on class_list that takes a constexpr std::array as a specialization, 
   // requiring one array element per type. The array elements serve as descriptive 
   // data which can be held alongside the type:
   // 
   //  - The (for_each) and (for_each_breakable) functions receive a type's matching 
   //    array element as an argument.
   // 
   //  - The (for_each_with_args) and (for_each_breakable_with_args) functions are 
   //    given the array element as an argument, followed by forwarded arguments.
   //
   template<auto list = impl::class_list_with_data::dummy_array, typename... Types> requires cobb::is_std_array_instance<list>
   class class_list_with_data {
      public:
         using data_type = typename decltype(list)::value_type;
         static_assert(list.size() == sizeof...(Types));

         static constexpr size_t count = sizeof...(Types);
         static constexpr size_t size() noexcept { return count; }

      private:
         template<int N, typename... Ts> using _nth_type = typename std::tuple_element<N, std::tuple<Ts...>>::type;

      public:
         template<template<typename T> typename functor, size_t n = 0> requires class_list_with_data_concepts::is_for_each_functor<functor, data_type>
         static void for_each() {
            functor<_nth_type<n, Types...>>::execute(list[n]);
            if constexpr (n + 1 < sizeof...(Types))
               for_each<functor, n + 1>();
         }

         template<template<typename T> typename functor, size_t n = 0> requires class_list_with_data_concepts::is_for_each_functor_with_bool_result<functor, data_type>
         static bool for_each_breakable() {
            if (functor<_nth_type<n, Types...>>::execute(list[n]))
               return true;
            if constexpr (n + 1 < sizeof...(Types))
               return for_each_breakable<functor, n + 1>();
            return false;
         }

         template<template<typename T> typename functor, size_t n = 0, typename... Args>
         static void for_each_with_args(Args&&... a) {
            functor<_nth_type<n, Types...>>::execute(list[n], std::forward<Args>(a)...);
            if constexpr (n + 1 < sizeof...(Types))
               for_each_with_args<functor, n + 1>(std::forward<Args>(a)...);
         }
         
         template<template<typename T> typename functor, size_t n = 0, typename... Args>
         static bool for_each_breakable_with_args(Args&&... a) {
            if (functor<_nth_type<n, Types...>>::execute(list[n], std::forward<Args>(a)...))
               return true;
            if constexpr (n + 1 < sizeof...(Types))
               return for_each_breakable_with_args<functor, n + 1>(std::forward<Args>(a)...);
            return false;
         }
   };
}