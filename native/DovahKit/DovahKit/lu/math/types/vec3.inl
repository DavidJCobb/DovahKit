#pragma once
#include "./vec3.h"
#include <version>
#include "../sqrt.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T> requires std::is_arithmetic_v<T>
#define CLASS_NAME vec3<T>

namespace lu::math {
   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::cross(const vec3& other) const noexcept {
      vec3 result;
      result.x = y * other.z - z * other.y;
      result.y = z * other.x - x * other.z;
      result.z = x * other.y - y * other.x;
      return result;
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::dot(const vec3& other) const noexcept {
      return (this->x * other.x) + (this->y * other.y) + (this->z * other.z);
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::isnan() const noexcept requires std::is_floating_point_v<value_type> {
      #if __cpp_lib_constexpr_cmath < 202202L
      if (std::is_constant_evaluated()) {
         if (this->x != this->x)
            return true;
         if (this->y != this->y)
            return true;
         if (this->z != this->z)
            return true;
         return false;
      } else {
      #endif
         return std::isnan(this->x) || std::isnan(this->y) || std::isnan(this->z);
      #if __cpp_lib_constexpr_cmath < 202202L
      }
      #endif
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::length_sq() const noexcept {
      return this->dot(*this);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::length() const noexcept {
      return static_cast<value_type>(sqrt(this->length_sq()));
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::normalized() const noexcept {
      return vec3(*this).normalize();
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::projected(const vec3& other) const noexcept {
      return other * (this->dot(other) / other.dot(other));
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::projected_onto_axis(const vec3& other) const noexcept {
      return other * this->dot(other);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::square() const noexcept {
      return this->length_sq();
   }

   #pragma region Operator overloads
      TEMPLATE_PARAMS
      constexpr const CLASS_NAME::value_type& CLASS_NAME::operator[](size_t i) const noexcept {
         if (std::is_constant_evaluated()) {
            switch (i) {
               case 0: return x;
               case 1: return y;
               case 2: return z;
               default:
                  // don't warn about `throw` in `noexcept`; we're using it to make constant evaluation fail deliberately
                  #pragma warning(suppress:4297)
                  throw;
            }
         } else {
            static_assert(offsetof(vec3, y) == offsetof(vec3, x) + sizeof(x), "Proper behavior of non-constexpr operator[] requires no padding between components.");
            static_assert(offsetof(vec3, z) == offsetof(vec3, y) + sizeof(y), "Proper behavior of non-constexpr operator[] requires no padding between components.");
            return std::addressof(this->x)[i];
         }
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::value_type& CLASS_NAME::operator[](size_t i) noexcept {
         if (std::is_constant_evaluated()) {
            switch (i) {
               case 0: return x;
               case 1: return y;
               case 2: return z;
               default:
                  // don't warn about `throw` in `noexcept`; we're using it to make constant evaluation fail deliberately
                  #pragma warning(suppress:4297)
                  throw;
            }
         } else {
            static_assert(offsetof(vec3, y) == offsetof(vec3, x) + sizeof(x), "Proper behavior of non-constexpr operator[] requires no padding between components.");
            static_assert(offsetof(vec3, z) == offsetof(vec3, y) + sizeof(y), "Proper behavior of non-constexpr operator[] requires no padding between components.");
            return std::addressof(this->x)[i];
         }
      }
         
      TEMPLATE_PARAMS
      constexpr CLASS_NAME CLASS_NAME::operator-() const noexcept {
         vec3 result = *this;
         result *= -1;
         return result;
      }
      
      #pragma region Vector-with-vector operators
         TEMPLATE_PARAMS
         template<typename O>
         constexpr CLASS_NAME& CLASS_NAME::operator+=(const O& other) noexcept requires vec3_like<O, value_type> {
            if (this == &other) {
               return (*this *= 2);
            }
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
         }

         TEMPLATE_PARAMS
         template<typename O>
         constexpr CLASS_NAME& CLASS_NAME::operator-=(const O& other) noexcept requires vec3_like<O, value_type> {
            if (this == &other) {
               *this = { 0, 0, 0 };
               return *this;
            }
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
         }

         TEMPLATE_PARAMS
         template<typename O>
         constexpr CLASS_NAME CLASS_NAME::operator+(const O& other) const noexcept requires vec3_like<O, value_type> {
            vec3 result = *this;
            result += other;
            return result;
         }

         TEMPLATE_PARAMS
         template<typename O>
         constexpr CLASS_NAME CLASS_NAME::operator-(const O& other) const noexcept requires vec3_like<O, value_type> {
            vec3 result = *this;
            result -= other;
            return result;
         }
      #pragma endregion
      #pragma region Vector-with-scalar operators
         #pragma region modify self
            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME& CLASS_NAME::operator+=(U other) noexcept {
               x += other;
               y += other;
               z += other;
               return *this;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME& CLASS_NAME::operator-=(U other) noexcept {
               x -= other;
               y -= other;
               z -= other;
               return *this;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME& CLASS_NAME::operator*=(U other) noexcept {
               x *= other;
               y *= other;
               z *= other;
               return *this;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME& CLASS_NAME::operator/=(U other) noexcept {
               x /= other;
               y /= other;
               z /= other;
               return *this;
            }
         #pragma endregion
         #pragma region create new
            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME CLASS_NAME::operator+(U other) const noexcept {
               vec3 result = *this;
               result += other;
               return result;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME CLASS_NAME::operator-(U other) const noexcept {
               vec3 result = *this;
               result -= other;
               return result;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME CLASS_NAME::operator*(U other) const noexcept {
               vec3 result = *this;
               result *= other;
               return result;
            }

            TEMPLATE_PARAMS
            template<typename U> requires std::is_arithmetic_v<U>
            constexpr CLASS_NAME CLASS_NAME::operator/(U other) const noexcept {
               vec3 result = *this;
               result /= other;
               return result;
            }
         #pragma endregion
      #pragma endregion
   #pragma endregion
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")