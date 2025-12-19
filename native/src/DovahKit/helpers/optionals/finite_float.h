#pragma once
#include <compare>
#include <limits>
#include <optional> // for std::nullopt_t
#include <type_traits>
#include <utility> // std::hash

namespace cobb::optionals {
   class finite_float {
      public:
         using value_type = float;

      protected:
         value_type _value = std::numeric_limits<value_type>::quiet_NaN();

      public:
         #pragma region Constructors
            constexpr finite_float() {}
            constexpr explicit finite_float(std::nullopt_t) {}
            constexpr finite_float(const finite_float& v) : _value(v._value) {}
            constexpr finite_float(finite_float&& v) : _value(std::move(v._value)) {}

            template<typename T> requires std::is_arithmetic_v<T>
            constexpr explicit(std::is_same_v<T, bool>) finite_float(T v) : _value(v) {}

            template<typename T> requires std::is_arithmetic_v<T>
            constexpr explicit(std::is_same_v<T, bool>) finite_float(const std::optional<T>& v) {
               if (v.has_value())
                  this->_value = v.value();
            }
         #pragma endregion
         #pragma region Assignment operators
            constexpr finite_float& operator=(const finite_float& other) noexcept;
            constexpr finite_float& operator=(finite_float&& other) noexcept;
            constexpr finite_float& operator=(value_type v) noexcept;

            template<typename T> requires std::is_arithmetic_v<T>
            constexpr finite_float& operator=(const std::optional<T>& v) {
               if (v.has_value())
                  this->_value = v.value();
               else
                  this->reset();
               return *this;
            }
         #pragma endregion
         #pragma region Observers
            constexpr const value_type* operator->() const noexcept;
            constexpr value_type* operator->() noexcept;
            constexpr const value_type& operator*() const noexcept;
            constexpr value_type& operator*() noexcept;

            constexpr bool has_value() const noexcept;
            constexpr const value_type& value() const noexcept;
            constexpr value_type& value() noexcept;

            constexpr value_type value_or(value_type v) const noexcept;
         #pragma endregion
         #pragma region Monadic operations
            template<typename Functor> constexpr auto and_then(Functor&  f) &;
            template<typename Functor> constexpr auto and_then(Functor&& f) const &;
            template<typename Functor> constexpr auto and_then(Functor&& f) &&;
            template<typename Functor> constexpr auto and_then(Functor&& f) const &&;

            // Analogue std::optional<T>::transform not provided because I can't decide 
            // on the best way to let the functor pick whether to return an optional<T> 
            // or another finite_float.

            template<typename Functor> constexpr finite_float or_else(Functor&& f) const &;
            template<typename Functor> constexpr finite_float or_else(Functor&& f) &&;
         #pragma endregion
         #pragma region Modifiers
            constexpr void swap(finite_float& other) noexcept;
            constexpr void reset();
            constexpr value_type& emplace(value_type) noexcept;
         #pragma endregion

   };
}

template<std::three_way_comparable_with<float> T>
constexpr std::compare_three_way_result_t<float, T> operator<=>(const cobb::optionals::finite_float& lhs, const std::optional<T>& rhs) {
   using result_type = std::compare_three_way_result_t<float, T>;
   if (lhs.has_value()) {
      if (rhs.has_value())
         return lhs.value() <=> rhs.value();
      return result_type::greater;
   } else {
      if (!rhs.has_value())
         return result_type::equal;
      return result_type::less;
   }
}

constexpr std::strong_ordering operator<=>(const cobb::optionals::finite_float& opt, std::nullopt_t) noexcept {
   if (opt.has_value())
      return std::strong_ordering::greater;
   return std::strong_ordering::equal;
}

template<std::three_way_comparable_with<float> Value>
constexpr std::compare_three_way_result_t<float, Value> operator<=>(const cobb::optionals::finite_float& opt, const Value& value) {
   if (opt.has_value())
      return opt.value() <=> value;
   return std::compare_three_way_result_t<float, Value>::less;
}

template<> struct std::hash<cobb::optionals::finite_float> {
   std::size_t operator()(const cobb::optionals::finite_float& s) const noexcept {
      return std::hash<cobb::optionals::finite_float::value_type>{}(s.value());
   }
};

#include "./finite_float.inl"