#pragma once
#include <type_traits>

namespace lu::utils {
   template<typename Self, typename Integral>
      requires std::is_integral_v<Integral>
   class alignas(sizeof(Integral)) integral_wrapper {
      public:
         using integral_type = Integral;

         integral_type wrapped_integral = {};

         constexpr integral_type as_integral() const noexcept {
            return this->wrapped_integral;
         }
         constexpr void set_from_integral(integral_type v) noexcept {
            this->wrapped_integral = v;
         }
   };

   namespace impl {
      template<typename T>
      concept has_integral_wrapper_interface = requires(T& v, const T& cv) {
         { cv.as_integral() };
         { v.set_from_integral(cv.as_integral()) };
      };

      template<has_integral_wrapper_interface T>
      using integral_wrapped_type = decltype(std::declval<T>().as_integral());
   }

   template<typename T>
   concept integral_wrapper_type = requires {
      requires impl::has_integral_wrapper_interface<T>;
      requires std::is_base_of_v<integral_wrapper<T, impl::integral_wrapped_type<T>>, T>;
   };

   template<integral_wrapper_type T>
   using integral_wrapper_underlying_type = impl::integral_wrapped_type<T>;
}

template<lu::utils::integral_wrapper_type T>
struct std::hash<T> : public std::hash<lu::utils::integral_wrapper_underlying_type<T>> {
};