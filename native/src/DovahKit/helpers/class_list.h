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
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>

namespace cobb {
   namespace class_list_concepts {
      template<template<typename T> typename functor> concept IsForEachFunctor = requires {
         { functor<void>::execute() };
      };
      template<template<typename T> typename functor> concept IsBreakableForEachFunctor = requires {
         { functor<void>::execute() } -> std::same_as<bool>;
      };
      template<template<typename T> typename functor> concept IsIndexOfMatchingFunctor = requires {
         { functor<void>::execute() } -> std::same_as<bool>;
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
   template<typename... Types> class class_list {
      public:
         static constexpr size_t count = sizeof...(Types);
         static constexpr size_t size() noexcept { return count; }

         using as_tuple = std::tuple<Types...>;

         template<size_t n> using nth_type = typename std::tuple_element<n, as_tuple>::type;

         template<typename T> static constexpr bool contains = (std::is_same_v<T, Types> || ...);

         template<typename T, size_t n = 0>
         static consteval size_t index_of() {
            if constexpr (std::is_same_v<T, nth_type<n>>)
               return n;
            if constexpr (n + 1 < count)
               return index_of<T, n + 1>();
            return -1;
         };

         template<template<typename T> typename functor, size_t n = 0> requires class_list_concepts::IsIndexOfMatchingFunctor<functor>
         static constexpr size_t index_of_matching() {
            if (functor<nth_type<n>>::execute())
               return n;
            if constexpr (n + 1 < count)
               return index_of_matching<functor, n + 1>();
            return -1;
         }

         template<template<typename T> typename functor> requires class_list_concepts::IsIndexOfMatchingFunctor<functor>
         static constexpr bool has_matching() {
            return index_of_matching<functor> != (size_t)-1;
         }

         template<template<typename T> typename functor, size_t n = 0> requires class_list_concepts::IsForEachFunctor<functor>
         static constexpr void for_each() {
            functor<nth_type<n>>::execute();
            if constexpr (n + 1 < count)
               for_each<functor, n + 1>();
         }

         template<template<typename T> typename functor, size_t n = 0> requires class_list_concepts::IsBreakableForEachFunctor<functor>
         static constexpr bool for_each_breakable() {
            if (functor<nth_type<n>>::execute())
               return true;
            if constexpr (n + 1 < count)
               return for_each_breakable<functor, n + 1>();
            return false;
         }

         template<template<typename T> typename functor, size_t n = 0, typename... Args>
         static constexpr void for_each_with_args(Args&&... a) {
            functor<nth_type<n>>::execute(std::forward<Args>(a)...);
            if constexpr (n + 1 < count)
               for_each_with_args<functor, n + 1>(std::forward<Args>(a)...);
         }
         
         template<template<typename T> typename functor, size_t n = 0, typename... Args>
         static constexpr bool for_each_breakable_with_args(Args&&... a) {
            if (functor<nth_type<n>>::execute(std::forward<Args>(a)...))
               return true;
            if constexpr (n + 1 < count)
               return for_each_breakable_with_args<functor, n + 1>(std::forward<Args>(a)...);
            return false;
         }

         template<template<typename T> typename functor, typename U, size_t n = 0>
         static constexpr auto for_each_with_previous(U prev = U()) -> U {
            U result = functor<nth_type<n>>::execute(prev);
            if constexpr (n + 1 < count)
               return for_each_with_previous<functor, U, n + 1>(result);
            return result;
         }
   };

   // ---

   namespace class_list_with_data_concepts {
      template<template<typename T> typename functor, typename data_type> concept IsForEachFunctor = requires (data_type v) {
         { functor<void>::execute(v) };
      };
      template<template<typename T> typename functor, typename data_type> concept IsBreakableForEachFunctor = requires (data_type v) {
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
         template<template<typename T> typename functor, size_t n = 0> requires class_list_with_data_concepts::IsForEachFunctor<functor, data_type>
         static void for_each() {
            functor<_nth_type<n, Types...>>::execute(list[n]);
            if constexpr (n + 1 < sizeof...(Types))
               for_each<functor, n + 1>();
         }

         template<template<typename T> typename functor, size_t n = 0> requires class_list_with_data_concepts::IsBreakableForEachFunctor<functor, data_type>
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