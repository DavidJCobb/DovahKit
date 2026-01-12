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
#include "./vector3.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename T> requires std::is_arithmetic_v<T>
#define CLASS_NAME vector3<T>

namespace cobb {
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type& CLASS_NAME::operator[](size_t i) noexcept {
      if (std::is_constant_evaluated()) {
         switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default:
               #pragma warning(suppress:4297) // throw-in-noexcept is intentional, to halt constant evaluation
               throw;
         }
      } else {
         return std::addressof(this->x)[i];
      }
   }

   TEMPLATE_PARAMS
   constexpr const CLASS_NAME::value_type& CLASS_NAME::operator[](size_t i) const noexcept {
      if (std::is_constant_evaluated()) {
         switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default:
               #pragma warning(suppress:4297) // throw-in-noexcept is intentional, to halt constant evaluation
               throw;
         }
      } else {
         return std::addressof(this->x)[i];
      }
   }

   #pragma region vector operations, non-destructive
   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::cross(const vector3& other) const noexcept {
      vector3 result;
      result.x = y * other.z - z * other.y;
      result.y = z * other.x - x * other.z;
      result.z = x * other.y - y * other.x;
      return result;
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::dot(const vector3& other) const noexcept {
      return (this->x * other.x) + (this->y * other.y) + (this->z * other.z);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::length_sq() const noexcept {
      return this->dot(*this);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::length() const noexcept {
      return ::cobb::sqrt(this->length_sq());
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::normalized() const noexcept {
      return vector3(*this).normalize();
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::projected(const vector3& other) const noexcept {
      return other * (this->dot(other) / other.dot(other));
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::projected_onto_axis(const vector3& other) const noexcept {
      return other * (this->dot(other));
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::value_type CLASS_NAME::square() const noexcept {
      return this->length_sq();
   }
   #pragma endregion
   
   #pragma region vector-with-vector operators
   //
   // NOTE: Using `vector3_like<typename CLASS_NAME::value_type>` here confuses IntelliSense. 
   //       Referencing the template parameter directly works.
   //
   TEMPLATE_PARAMS
   template<typename O>
   constexpr CLASS_NAME& CLASS_NAME::operator+=(const O& other) noexcept requires vector3_like<O, value_type> {
      x += other.x;
      y += other.y;
      z += other.z;
      return *this;
   }

   TEMPLATE_PARAMS
   template<typename O>
   constexpr CLASS_NAME& CLASS_NAME::operator-=(const O& other) noexcept requires vector3_like<O, value_type> {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      return *this;
   }

   TEMPLATE_PARAMS
   template<typename O>
   constexpr CLASS_NAME CLASS_NAME::operator+(const O& other) const noexcept requires vector3_like<O, value_type> {
      vector3 result = *this;
      result += other;
      return result;
   }

   TEMPLATE_PARAMS
   template<typename O>
   constexpr CLASS_NAME CLASS_NAME::operator-(const O& other) const noexcept requires vector3_like<O, value_type> {
      vector3 result = *this;
      result -= other;
      return result;
   }
   #pragma endregion
   
   #pragma region vector-with-scalar operators
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
            vector3 result = *this;
            result += other;
            return result;
         }

         TEMPLATE_PARAMS
         template<typename U> requires std::is_arithmetic_v<U>
         constexpr CLASS_NAME CLASS_NAME::operator-(U other) const noexcept {
            vector3 result = *this;
            result -= other;
            return result;
         }

         TEMPLATE_PARAMS
         template<typename U> requires std::is_arithmetic_v<U>
         constexpr CLASS_NAME CLASS_NAME::operator*(U other) const noexcept {
            vector3 result = *this;
            result *= other;
            return result;
         }

         TEMPLATE_PARAMS
         template<typename U> requires std::is_arithmetic_v<U>
         constexpr CLASS_NAME CLASS_NAME::operator/(U other) const noexcept {
            vector3 result = *this;
            result /= other;
            return result;
         }
      #pragma endregion
   #pragma endregion
         
   TEMPLATE_PARAMS
   constexpr CLASS_NAME CLASS_NAME::operator-() const noexcept {
      vector3 result = *this;
      result *= -1;
      return result;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::operator==(const vector3& other) const noexcept {
      if (this->x != other.x)
         return false;
      if (this->y != other.y)
         return false;
      if (this->z != other.z)
         return false;
      return true;
   }
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")