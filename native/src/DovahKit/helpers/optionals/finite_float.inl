#pragma once
#include "./finite_float.h"
#include <cmath>
#include <version>
#if __cpp_lib_constexpr_cmath < 202202L
   #include <bit>
#endif

namespace cobb::optionals {
   #pragma region Assignment operators
      constexpr finite_float& finite_float::operator=(const finite_float& other) noexcept {
         this->_value = other._value; return *this;
      }

      constexpr finite_float& finite_float::operator=(finite_float&& other) noexcept {
         this->_value = other._value;
         other = {};
         return *this;
      }

      constexpr finite_float& finite_float::operator=(value_type v) noexcept {
         this->_value = v;
         return *this;
      }
   #pragma endregion
   #pragma region Observers
      constexpr const finite_float::value_type* finite_float::operator->() const noexcept {
         return &this->_value;
      }
      constexpr finite_float::value_type* finite_float::operator->() noexcept {
         return &this->_value;
      }
      constexpr const finite_float::value_type& finite_float::operator*() const noexcept {
         return this->_value;
      }
      constexpr finite_float::value_type& finite_float::operator*() noexcept {
         return this->_value;
      }

      constexpr bool finite_float::has_value() const noexcept {
         if (!std::is_constant_evaluated()) {
            return std::isfinite(this->_value);
         } else {
            #if __cpp_lib_constexpr_cmath >= 202202L
               return std::isfinite(this->_value);
            #else
               constexpr const uint32_t nan_bits = std::bit_cast<uint32_t>(std::numeric_limits<value_type>::quiet_NaN());
               constexpr const uint32_t inf_pos  = std::bit_cast<uint32_t>(std::numeric_limits<value_type>::infinity());
               constexpr const uint32_t inf_neg  = std::bit_cast<uint32_t>(-std::numeric_limits<value_type>::infinity());
               switch (std::bit_cast<uint32_t>(this->_value)) {
                  case nan_bits:
                  case inf_pos:
                  case inf_neg:
                     return false;
               }
               return true;
            #endif
         }
      }
      constexpr const finite_float::value_type& finite_float::value() const noexcept { return this->_value; }
      constexpr finite_float::value_type& finite_float::value() noexcept { return this->_value; }

      constexpr finite_float::value_type finite_float::value_or(value_type v) const noexcept {
         if (this->has_value())
            return this->value();
         return v;
      }
   #pragma endregion
   #pragma region Monadic operations
      template<typename Functor> constexpr auto finite_float::and_then(Functor& f) & {
         if (*this)
            return std::invoke(std::forward<Functor>(f), this->value());
         else
            return std::remove_cvref_t<std::invoke_result_t<Functor, value_type&>>{};
      }
      template<typename Functor> constexpr auto finite_float::and_then(Functor&& f) const & {
         if (*this)
            return std::invoke(std::forward<Functor>(f), this->value());
         else
            return std::remove_cvref_t<std::invoke_result_t<Functor, const value_type&>>{};
      }
      template<typename Functor> constexpr auto finite_float::and_then(Functor&& f) && {
         if (*this)
            return std::invoke(std::forward<Functor>(f), std::move(this->value()));
         else
            return std::remove_cvref_t<std::invoke_result_t<Functor, value_type>>{};
      }
      template<typename Functor> constexpr auto finite_float::and_then(Functor&& f) const && {
         if (*this)
            return std::invoke(std::forward<Functor>(f), std::move(this->value()));
         else
            return std::remove_cvref_t<std::invoke_result_t<Functor, const value_type>>{};
      }

      template<typename Functor> constexpr finite_float finite_float::or_else(Functor&& f) const & {
         return this->has_value() ? *this : std::forward<Functor>(f)();
      }
      template<typename Functor> constexpr finite_float finite_float::or_else(Functor&& f) && {
         return this->has_value() ? std::move(*this) : std::forward<Functor>(f)();
      }
   #pragma endregion
   #pragma region Modifiers
      constexpr void finite_float::swap(finite_float& other) noexcept {
         std::swap(this->_value, other._value);
      }
      constexpr void finite_float::reset() {
         this->_value = std::numeric_limits<value_type>::quiet_NaN();
      }
      constexpr finite_float::value_type& finite_float::emplace(value_type v) noexcept {
         return (this->_value = v);
      }
   #pragma endregion
}