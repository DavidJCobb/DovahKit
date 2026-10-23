#pragma once
#include <limits>
#include "./integral_wrapper.h"
#include "../macros/FORCE_INLINE.define.h"

namespace lu::utils {
   template<typename Self, typename Integral>
      requires std::is_integral_v<Integral>
   class alignas(Integral) nominal_integer_type : public integral_wrapper<Self, Integral> {
      private:
         using _my_base = integral_wrapper<Self, Integral>;
         using _self    = Self;
      public:
         using integral_type = _my_base::integral_type;
         using _my_base::as_integral;
         using _my_base::set_from_integral;

      public:
         LU_FORCE_INLINE constexpr nominal_integer_type() {}

         template<typename U>
            requires (std::is_arithmetic_v<U> || std::is_integral_v<U>)
         LU_FORCE_INLINE constexpr nominal_integer_type(U v) {
            this->wrapped_integral = v;
         }

         template<typename Identity, typename U>
         LU_FORCE_INLINE constexpr nominal_integer_type(nominal_integer_type<Identity, U> v) {
            this->wrapped_integral = (U)v;
         }

      public:
         #pragma region Arithmetic and binary operators
            #pragma push_macro(MAKE_ARITH_OPERATOR_OVERLOAD)
            #define MAKE_ARITH_OPERATOR_OVERLOAD(_op_) \
               template<typename U> \
                  requires (std::is_arithmetic_v<U>) \
               LU_FORCE_INLINE constexpr Self& operator _op_##= (this Self& self, U v) noexcept { \
                  self.wrapped_integral _op_##= v; \
                  return self; \
               } \
               template<typename U> \
                  requires (std::is_arithmetic_v<U>) \
               LU_FORCE_INLINE constexpr Self operator _op_ (this const Self& self, U v) noexcept { \
                  auto out = self; out _op_##= v; return out; \
               } \
               template<typename Identity, typename U> \
               LU_FORCE_INLINE constexpr Self& operator _op_##= (this Self& self, nominal_integer_type<Identity, U> v) noexcept { \
                  return self _op_##= v.wrapped_integral; \
               } \
               template<typename Identity, typename U> \
               LU_FORCE_INLINE constexpr Self operator _op_ (this const Self& self, nominal_integer_type<Identity, U> v) noexcept { \
                  return self _op_ v.wrapped_integral; \
               }

            MAKE_ARITH_OPERATOR_OVERLOAD(+);
            MAKE_ARITH_OPERATOR_OVERLOAD(-);
            MAKE_ARITH_OPERATOR_OVERLOAD(*);
            MAKE_ARITH_OPERATOR_OVERLOAD(/);
            MAKE_ARITH_OPERATOR_OVERLOAD(%);
            MAKE_ARITH_OPERATOR_OVERLOAD(>>);
            MAKE_ARITH_OPERATOR_OVERLOAD(<<);
            MAKE_ARITH_OPERATOR_OVERLOAD(&);
            MAKE_ARITH_OPERATOR_OVERLOAD(|);
            MAKE_ARITH_OPERATOR_OVERLOAD(^);

            #undef MAKE_ARITH_OPERATOR_OVERLOAD
            #pragma pop_macro(MAKE_ARITH_OPERATOR_OVERLOAD)
         #pragma endregion

         #pragma region Comparison operator overloads
            #pragma push_macro(MAKE_COMPARISON_OPERATOR_OVERLOAD)
            #define MAKE_COMPARISON_OPERATOR_OVERLOAD(_op_) \
               template<typename U> \
                  requires (std::is_arithmetic_v<U>) \
               LU_FORCE_INLINE constexpr bool operator _op_(U v) const noexcept { \
                  return this->wrapped_integral _op_ v; \
               } \
               template<typename Identity, typename U> \
               LU_FORCE_INLINE constexpr bool operator _op_ (nominal_integer_type<Identity, U> v) const noexcept { \
                  return *this _op_ v.wrapped_integral; \
               }

            MAKE_COMPARISON_OPERATOR_OVERLOAD(<);
            MAKE_COMPARISON_OPERATOR_OVERLOAD(<=);
            MAKE_COMPARISON_OPERATOR_OVERLOAD(>);
            MAKE_COMPARISON_OPERATOR_OVERLOAD(>=);
            MAKE_COMPARISON_OPERATOR_OVERLOAD(==);
            MAKE_COMPARISON_OPERATOR_OVERLOAD(!=);

            #undef MAKE_COMPARISON_OPERATOR_OVERLOAD
            #pragma pop_macro(MAKE_COMPARISON_OPERATOR_OVERLOAD)
         #pragma endregion

         #pragma region Unary operator overloads
            LU_FORCE_INLINE constexpr Self operator+(this const Self& self) noexcept {
               return self;
            }
            LU_FORCE_INLINE constexpr Self operator-(this const Self& self) noexcept requires std::is_signed_v<integral_type> {
               Self v = self;
               v.wrapped_integral = -v.wrapped_integral;
               return v;
            }
            LU_FORCE_INLINE constexpr Self operator!(this const Self& self) noexcept {
               return Self(self.wrapped_integral != 0);
            }
         #pragma endregion

         LU_FORCE_INLINE explicit constexpr operator bool() const noexcept {
            return this->wrapped_integral != 0;
         }

         template<typename U>
            requires (std::is_arithmetic_v<U> || std::is_integral_v<U>)
         LU_FORCE_INLINE explicit constexpr operator U () const noexcept {
            return (U)this->wrapped_integral;
         }

         template<typename Identity, typename U>
         LU_FORCE_INLINE explicit constexpr operator nominal_integer_type<Identity, U> () const noexcept {
            return U(this->wrapped_integral);
         }
   };

   namespace impl {
      template<typename T>
      concept is_nominal_integer_type = requires {
         requires has_integral_wrapper_interface<T>;
         requires std::is_base_of_v<nominal_integer_type<T, integral_wrapped_type<T>>, T>;
      };
   }
}

template<lu::utils::impl::is_nominal_integer_type T>
class std::numeric_limits<T> : public std::numeric_limits<lu::utils::impl::integral_wrapped_type<T>> {
};

#include "../macros/FORCE_INLINE.undef.h"

#define LU_DEFINE_NOMINAL_INTEGER_TYPE(name, underlying) \
   class name : public lu::utils::nominal_integer_type<name, underlying> { \
      private: \
         using _my_base = lu::utils::nominal_integer_type<name, underlying>; \
      public: \
         using integral_type = _my_base::integral_type; \
         using _my_base::nominal_integer_type; \
   }; \
   static_assert(sizeof(name)  == sizeof(underlying)); \
   static_assert(alignof(name) == alignof(underlying));