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
#include <math.h>
#include <type_traits>

namespace cobb {
   template<typename T> class vector3 {
      public:
         static_assert(std::is_arithmetic_v<T>, "cobb::vector3 must be templated on a numeric type.");
         static constexpr int axis_count = 3;
         
         union {
            struct {
               T x;
               T y;
               T z;
            };
            T components[axis_count] = { T(0), T(0), T(0) };
         };

         constexpr vector3() {}
         constexpr vector3(T x, T y, T z) : x(x), y(y), z(z) {}

         template<typename U> requires std::is_arithmetic_v<U>
         constexpr vector3(U x, U y, U z) : x(x), y(y), z(z) {}

         template<typename U> explicit constexpr vector3(const std::array<U, axis_count>& a) {
            static_assert(std::is_arithmetic_v<U>, "cobb::vector3 can only be constructed with an array if it's an array of a numeric type.");
            this->x = a[0];
            this->y = a[1];
            this->z = a[2];
         }

         template<typename U> constexpr operator std::array<U, axis_count>() const noexcept {
            static_assert(std::is_arithmetic_v<U>, "cobb::vector3 can only be converted to an array of a numeric type.");
            return { this->x, this->y, this->z };
         }
         [[nodiscard]] std::array<T, axis_count> to_array() const noexcept {
            return { this->x, this->y, this->z };
         }
         
         [[nodiscard]] vector3 cross(const vector3& other) const noexcept {
            vector3 result;
            result.x = (float)y * (float)other.z - (float)z * (float)other.y;
            result.y = (float)z * (float)other.x - (float)x * (float)other.z;
            result.z = (float)x * (float)other.y - (float)y * (float)other.x;
            return result;
         }
         [[nodiscard]] T dot(const vector3& other) const noexcept {
            /*//
            T sum = 0.0F;
            for (int i = 0; i < axis_count; ++i)
               sum += this->components[i] * other.components[i];
            return sum;
            //*/
            return (this->x * other.x) + (this->y * other.y) + (this->z * other.z);
         }
         [[nodiscard]] T length_sq() const noexcept {
            float a = x * x;
            float b = y * y;
            float c = z * z;
            return a + b + c;
         }
         [[nodiscard]] inline T length() const noexcept {
            return sqrt(this->length_sq());
         }
         vector3& normalize() noexcept {
            *this /= this->length();
            return *this;
         }
         [[nodiscard]] T square() const noexcept { // equivalent operation to taking the dot product of the vector with itself
            return this->length_sq();
         }
         
         #pragma region vector-with-vector operators
         vector3& operator+=(const vector3& other) noexcept {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
         }
         vector3& operator-=(const vector3& other) noexcept {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
         }
         vector3 operator+(const vector3& other) const noexcept {
            vector3 result = *this;
            result += other;
            return result;
         }
         vector3 operator-(const vector3& other) const noexcept {
            vector3 result = *this;
            result -= other;
            return result;
         }
         #pragma endregion

         #pragma region vector-with-scalar operators
            #pragma region modify self
            template<typename U> vector3& operator+=(U other) noexcept {
               x += other;
               y += other;
               z += other;
               return *this;
            }
            template<typename U> vector3& operator-=(U other) noexcept {
               x -= other;
               y -= other;
               z -= other;
               return *this;
            }
            template<typename U> vector3& operator*=(U other) noexcept {
               static_assert(std::is_arithmetic_v<U>, "cobb::vector3::operator*= must be given a numeric argument.");
               x *= other;
               y *= other;
               z *= other;
               return *this;
            }
            template<typename U> vector3& operator/=(U other) noexcept {
               static_assert(std::is_arithmetic_v<U>, "cobb::vector3::operator/= must be given a numeric argument.");
               x /= other;
               y /= other;
               z /= other;
               return *this;
            }
            #pragma endregion
            //
            #pragma region create new
            template<typename U> vector3 operator+(U other) const noexcept {
               vector3 result = *this;
               result += other;
               return result;
            }
            template<typename U> vector3 operator-(U other) const noexcept {
               vector3 result = *this;
               result -= other;
               return result;
            }
            template<typename U> vector3 operator*(U other) const noexcept {
               static_assert(std::is_arithmetic_v<U>, "cobb::vector3::operator* must be given a numeric argument.");
               vector3 result = *this;
               result *= other;
               return result;
            }
            template<typename U> vector3 operator/(U other) const noexcept {
               static_assert(std::is_arithmetic_v<U>, "cobb::vector3::operator/ must be given a numeric argument.");
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