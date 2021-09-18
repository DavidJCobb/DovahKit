#pragma once
#include <tuple>
#include <type_traits>

namespace cobb {
   //
   // Given a parameter pack of values, check if all values are of the same type. Useful 
   // for implementing a function that is meant to take a variable number of arguments of 
   // the same type, e.g.
   // 
   //    template<typename... ts> requires cobb::all_same<ts...>
   //    static constexpr int or_all(ts... M) {
   //       return (M | ...);
   //    }
   // 
   // If you wish to take a specific type, use all_same_as. The all_same concept here 
   // allows any type, so long as all of the parameters are of the same type as each other.
   //
   template<class... Ts> concept all_same =
      sizeof...(Ts) < 2 ||
      std::conjunction_v<std::is_same<std::tuple_element_t<0, std::tuple<Ts...>>, Ts>...>;
   
   //
   // Given a parameter pack of values, check if all values are of the same type. Useful 
   // for implementing a function that is meant to take a variable number of arguments of 
   // the same type, e.g.
   // 
   //    template<typename... ts> requires cobb::all_same_as<T, ts...>
   //    static constexpr int or_all(ts... M) {
   //       return (M | ...);
   //    }
   // 
   // You could also do the following, but it would allow arguments that are implicitly 
   // convertible to the desired type as well:
   // 
   //    template<int... ts>
   //    static constexpr int or_all(ts... M) {
   //       return (M | ...);
   //    }
   //
   template<typename T, class... Ts> concept all_same_as = requires {
      requires sizeof...(Ts) > 0;
      requires std::same_as<std::tuple_element_t<0, std::tuple<Ts...>>, T>;
      requires sizeof...(Ts) < 2 || std::conjunction_v<std::is_same<std::tuple_element_t<0, std::tuple<Ts...>>, Ts>...>;
   };

   //
   // There are only two ways to make a template take a std::array instance as a parameter:
   // 
   //    template<typename T, size_t i, std::array<T, i> values> struct foo {
   //       // ...
   //    };
   //    using my_foo = foo<int, 3, std::array{1, 2, 3}>;
   // 
   // Or:
   // 
   //    template<auto values> struct foo {
   //       // ...
   //    };
   //    using my_foo = foo<std::array{1, 2, 3}>;
   // 
   // In the latter case, one may wish to constrain the template and ensure that the passed-
   // in parameter is in fact an std::array; one can do so as:
   // 
   //    template<auto values> requires cobb::is_std_array_instance<values>
   //    struct foo {
   //       // ...
   //    };
   //
   template<auto value> concept is_std_array_instance = requires {
      typename std::decay<decltype(value)>::value_type;
      { value.size() } -> std::same_as<size_t>;
      std::is_same_v<std::decay<decltype(value)>, std::array<typename std::decay<decltype(value)>::value_type, value.size()>>;
   };
}