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
#include <type_traits>
#include "./math/sqrt.h"

namespace cobb {
   template<typename Struct, typename ValueType> concept vector3_like = requires(Struct x) {
      { x.x } -> std::convertible_to<ValueType>;
      { x.y } -> std::convertible_to<ValueType>;
      { x.z } -> std::convertible_to<ValueType>;
   };

   template<typename T = float> requires std::is_arithmetic_v<T>
   class vector3 {
      public:
         static constexpr const size_t axis_count = 3;
         using value_type = T;

         value_type x = T{0};
         value_type y = T{0};
         value_type z = T{0};

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

         template<vector3_like<value_type> O>
         constexpr vector3(const O& other) {
            this->x = other.x;
            this->y = other.y;
            this->z = other.z;
         }

         template<typename U> requires std::is_arithmetic_v<U>
         constexpr operator std::array<U, axis_count>() const noexcept {
            return { this->x, this->y, this->z };
         }

         std::array<value_type, axis_count> to_array() const noexcept {
            return { this->x, this->y, this->z };
         }

         template<vector3_like<value_type> O>
         constexpr O to_struct() const noexcept {
            O out;
            out.x = this->x;
            out.y = this->y;
            out.z = this->z;
            return out;
         }

         constexpr value_type& operator[](size_t i) noexcept;
         constexpr const value_type& operator[](size_t i) const noexcept;
         
         constexpr vector3 cross(const vector3& other) const noexcept;
         constexpr value_type dot(const vector3& other) const noexcept;
         constexpr value_type length_sq() const noexcept;
         constexpr value_type length() const noexcept;
         constexpr vector3& normalize() noexcept {
            *this /= this->length();
            return *this;
         }
         constexpr vector3 normalized() const noexcept;
         constexpr vector3 projected(const vector3& other) const noexcept;
         constexpr vector3 projected_onto_axis(const vector3& normalized_axis) const noexcept; // same as `projected` but assumes a normalized argument
         constexpr value_type square() const noexcept; // equivalent operation to taking the dot product of the vector with itself
         
         #pragma region vector-with-vector operators
         template<typename O> constexpr vector3& operator+=(const O& other) noexcept requires vector3_like<O, value_type>;
         template<typename O> constexpr vector3& operator-=(const O& other) noexcept requires vector3_like<O, value_type>;

         template<typename O> constexpr vector3 operator+(const O& other) const noexcept requires vector3_like<O, value_type>;
         template<typename O> constexpr vector3 operator-(const O& other) const noexcept requires vector3_like<O, value_type>;
         #pragma endregion

         #pragma region vector-with-scalar operators
            #pragma region modify self
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3& operator+=(U other) noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3& operator-=(U other) noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3& operator*=(U other) noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3& operator/=(U other) noexcept;
            #pragma endregion
            #pragma region create new
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3 operator+(U other) const noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3 operator-(U other) const noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3 operator*(U other) const noexcept;
               template<typename U> requires std::is_arithmetic_v<U> constexpr vector3 operator/(U other) const noexcept;
            #pragma endregion
         #pragma endregion

         constexpr vector3 operator-() const noexcept;

         constexpr bool operator==(const vector3& other) const noexcept;
   };

   template<typename Struct, typename ValueType = float> concept vector3_identical = requires(Struct x) {
      typename vector3<ValueType>;
      { x.x } -> std::same_as<ValueType>;
      { x.y } -> std::same_as<ValueType>;
      { x.z } -> std::same_as<ValueType>;
      #if __INTELLISENSE__
         // IntelliSense uses the Edison Design Group compiler, and the MSVC STL definition of 
         // std::is_layout_compatible[_v] is disabled when "__EDG__" is present. We therefore 
         // have to use the compiler intrinsic directly, or IntelliSense will puke.
         // 
         //    [12/3/2022] [still in effect as of 9/10/2023]
         // 
         requires __is_layout_compatible(Struct, vector3<ValueType>);
      #else
         requires std::is_layout_compatible_v<Struct, vector3<ValueType>>;
      #endif
   };

   template<typename Struct, typename ValueType = float> vector3<ValueType>& reinterpret_as_vector3(Struct& o) {
      return *(vector3<ValueType>*)(&o);
   }
   template<typename Struct, typename ValueType = float> const vector3<ValueType>& reinterpret_as_vector3(const Struct& o) {
      return *(const vector3<ValueType>*)(&o);
   }
};

#include "./vector3.inl"