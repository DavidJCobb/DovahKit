#pragma once
#include <type_traits>

namespace dovah {
   template<typename T, typename U = std::underlying_type_t<T>> class loose_enum {
      public:
         using value_type      = T;
         using underlying_type = U;
         static_assert(std::is_enum_v<value_type>,          "value_type must be an enum");
         static_assert(std::is_integral_v<underlying_type>, "underlying_type must be an integral");
      protected:
         T value;
      public:
         constexpr loose_enum() noexcept = default;
			constexpr loose_enum(const loose_enum&) noexcept = default;
			constexpr loose_enum(loose_enum&&) noexcept = default;
         constexpr loose_enum(value_type o) : value(o) {}
         constexpr loose_enum(underlying_type o) : value((value_type)o) {}

			template <class underlying_alt> constexpr loose_enum(loose_enum<value_type, underlying_alt> other) noexcept : value((underlying_type)other) {}

			~loose_enum() noexcept = default;

         #pragma region assignment operators
			constexpr loose_enum& operator=(const loose_enum&) noexcept = default;
			constexpr loose_enum& operator=(loose_enum&&) noexcept = default;
         inline loose_enum& operator=(const value_type o) noexcept {
            this->value = o;
            return *this;
         }
         inline loose_enum& operator=(const underlying_type o) noexcept {
            this->value = (value_type)o;
            return *this;
         }
         template <class underlying_alt> inline constexpr loose_enum& operator=(loose_enum<value_type, underlying_alt> a_rhs) noexcept {
            this->value = static_cast<underlying_type>(a_rhs.get());
            return *this;
         }
         #pragma endregion

         #pragma region increment and addition operators
         inline loose_enum& operator++() noexcept { // prefix
            this->value = (value_type)((underlying_type)this->value + 1);
            return *this;
         }
         inline loose_enum operator++(int) noexcept { // postfix
            auto old = *this;
            operator++();
            return old;
         }
         inline loose_enum operator+(underlying_type mod) const noexcept {
            loose_enum out = *this;
            out.value = (value_type)((underlying_type)out.value + mod);
            return out;
         }
         #pragma endregion

         #pragma region decrement and subtraction operators
         inline loose_enum& operator--() noexcept { // prefix
            this->value = (value_type)((underlying_type)this->value - 1);
            return *this;
         }
         inline loose_enum operator--(int) noexcept { // postfix
            auto old = *this;
            operator++();
            return old;
         }
         inline loose_enum operator-(underlying_type mod) const noexcept {
            loose_enum out = *this;
            out.value = (value_type)((underlying_type)out.value - mod);
            return out;
         }
         #pragma endregion

			[[nodiscard]] explicit inline constexpr operator bool() const noexcept { return (underlying_type)this->value != 0; }
         inline constexpr operator value_type() const noexcept { return this->value; }

         inline constexpr operator underlying_type() const noexcept {
            return (T)this->value;
         }

   };
}