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
#include <array>
#include <concepts>
#include <math.h>
#include <type_traits>

namespace cobb {
   namespace impl::vector3 {
      template<typename T, typename O> concept is_vector_like = requires(O x) {
         { x.x } -> std::convertible_to<T>;
         { x.y } -> std::convertible_to<T>;
         { x.z } -> std::convertible_to<T>;
      };
   }

   template<typename T = float> requires std::is_arithmetic_v<T>
   class vector3 {
      public:
         static constexpr int axis_count = 3;

         using value_type = T;
         
         union {
            struct {
               value_type x;
               value_type y;
               value_type z;
            };
            std::array<value_type, axis_count> components = { T(0), T(0), T(0) };
         };

         constexpr vector3() {}
         constexpr vector3(value_type x, value_type y, value_type z) : x(x), y(y), z(z) {}

         // Constructor to allow narrowing conversions (e.g. double -> float) without manual casts
         template<typename U, typename V, typename W> requires (std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W>)
         constexpr vector3(U x, V y, W z) : x(x), y(y), z(z) {}

         template<typename U> requires std::is_arithmetic_v<U>
         explicit constexpr vector3(const std::array<U, axis_count>& a) {
            this->x = a[0];
            this->y = a[1];
            this->z = a[2];
         }

         template<typename O> requires impl::vector3::is_vector_like<value_type, O>
         constexpr vector3(const O& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
         }

         template<typename U> requires std::is_arithmetic_v<U>
         constexpr operator std::array<U, axis_count>() const noexcept {
            return { this->x, this->y, this->z };
         }
         [[nodiscard]] std::array<value_type, axis_count> to_array() const noexcept {
            return { this->x, this->y, this->z };
         }

         template<typename O> requires impl::vector3::is_vector_like<value_type, O>
         constexpr O to_struct() const noexcept {
            O out;
            out.x = this->x;
            out.y = this->y;
            out.z = this->z;
            return out;
         }
         
         [[nodiscard]] vector3 cross(const vector3& other) const noexcept {
            vector3 result;
            result.x = y * other.z - z * other.y;
            result.y = z * other.x - x * other.z;
            result.z = x * other.y - y * other.x;
            return result;
         }
         [[nodiscard]] value_type dot(const vector3& other) const noexcept {
            return (this->x * other.x) + (this->y * other.y) + (this->z * other.z);
         }
         [[nodiscard]] value_type length_sq() const noexcept {
            value_type a = x * x;
            value_type b = y * y;
            value_type c = z * z;
            return a + b + c;
         }
         [[nodiscard]] inline value_type length() const noexcept {
            return sqrt(this->length_sq());
         }
         vector3& normalize() noexcept {
            *this /= this->length();
            return *this;
         }
         [[nodiscard]] vector3 normalized() const noexcept {
            return vector3(*this).normalize();
         }
         [[nodiscard]] value_type square() const noexcept { // equivalent operation to taking the dot product of the vector with itself
            return this->length_sq();
         }
         
         #pragma region vector-with-vector operators
         template<typename O> requires impl::vector3::is_vector_like<T, O>
         vector3& operator+=(const O& other) noexcept {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
         }
         template<typename O> requires impl::vector3::is_vector_like<T, O>
         vector3& operator-=(const O& other) noexcept {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
         }
         template<typename O> requires impl::vector3::is_vector_like<T, O>
         vector3 operator+(const O& other) const noexcept {
            vector3 result = *this;
            result += other;
            return result;
         }
         template<typename O> requires impl::vector3::is_vector_like<T, O>
         vector3 operator-(const O& other) const noexcept {
            vector3 result = *this;
            result -= other;
            return result;
         }
         #pragma endregion

         #pragma region vector-with-scalar operators
            #pragma region modify self
            template<typename U> requires std::is_arithmetic_v<U> vector3& operator+=(U other) noexcept {
               x += other;
               y += other;
               z += other;
               return *this;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3& operator-=(U other) noexcept {
               x -= other;
               y -= other;
               z -= other;
               return *this;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3& operator*=(U other) noexcept {
               x *= other;
               y *= other;
               z *= other;
               return *this;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3& operator/=(U other) noexcept {
               x /= other;
               y /= other;
               z /= other;
               return *this;
            }
            #pragma endregion
            //
            #pragma region create new
            template<typename U> requires std::is_arithmetic_v<U> vector3 operator+(U other) const noexcept {
               vector3 result = *this;
               result += other;
               return result;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3 operator-(U other) const noexcept {
               vector3 result = *this;
               result -= other;
               return result;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3 operator*(U other) const noexcept {
               vector3 result = *this;
               result *= other;
               return result;
            }
            template<typename U> requires std::is_arithmetic_v<U> vector3 operator/(U other) const noexcept {
               vector3 result = *this;
               result /= other;
               return result;
            }
            #pragma endregion
         #pragma endregion

         vector3 operator-() const noexcept {
            vector3 result = *this;
            result *= -1;
            return result;
         }

         #pragma region Comparisons
         bool operator==(const vector3& other) const noexcept {
            if (this->x != other.x)
               return false;
            if (this->y != other.y)
               return false;
            if (this->z != other.z)
               return false;
            return true;
         }
         bool operator!=(const vector3& other) const noexcept {
            if (this->x == other.x)
               return false;
            if (this->y == other.y)
               return false;
            if (this->z == other.z)
               return false;
            return true;
         }
         #pragma endregion
   };

};