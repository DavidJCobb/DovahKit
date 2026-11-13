#pragma once
#include <array>
#include <concepts>
#include <type_traits>

namespace lu::math {
   template<typename T, typename ValueType> 
   concept vec3_like = requires(const T& x) {
      { x.x } -> std::convertible_to<ValueType>;
      { x.y } -> std::convertible_to<ValueType>;
      { x.z } -> std::convertible_to<ValueType>;
   };

   template<typename T = float> requires std::is_arithmetic_v<T>
   class vec3 {
      public:
         static constexpr const size_t axis_count = 3;
         using value_type = T;

         value_type x = value_type{0};
         value_type y = value_type{0};
         value_type z = value_type{0};

      public:
         constexpr vec3 cross(const vec3& other) const noexcept;
         constexpr value_type dot(const vec3& other) const noexcept;
         constexpr bool isnan() const noexcept requires std::is_floating_point_v<value_type>; // true if any component is NaN
         constexpr value_type length_sq() const noexcept;
         constexpr value_type length() const noexcept;
         constexpr vec3& normalize() noexcept {
            *this /= this->length();
            return *this;
         }
         constexpr vec3 normalized() const noexcept;
         constexpr vec3 projected(const vec3& other) const noexcept;
         constexpr vec3 projected_onto_axis(const vec3& normalized_axis) const noexcept; // same as `projected` but assumes a normalized argument
         constexpr value_type square() const noexcept; // equivalent operation to taking the dot product of the vector with itself

         #pragma region Conversions
            constexpr std::array<value_type, axis_count> to_array() const noexcept {
               return std::array{ x, y, z };
            }
         #pragma endregion

         #pragma region Operator overloads
            constexpr bool operator==(const vec3& other) const noexcept = default;

            // Component access by index
            constexpr const value_type& operator[](size_t) const noexcept;
            constexpr value_type& operator[](size_t) noexcept;

            constexpr vec3 operator-() const noexcept;

            #pragma region vector-with-vector operators
               template<typename O> constexpr vec3& operator+=(const O& other) noexcept requires vec3_like<O, value_type>;
               template<typename O> constexpr vec3& operator-=(const O& other) noexcept requires vec3_like<O, value_type>;

               template<typename O> constexpr vec3 operator+(const O& other) const noexcept requires vec3_like<O, value_type>;
               template<typename O> constexpr vec3 operator-(const O& other) const noexcept requires vec3_like<O, value_type>;
            #pragma endregion
            #pragma region vector-with-scalar operators
               #pragma region modify self
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3& operator+=(U other) noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3& operator-=(U other) noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3& operator*=(U other) noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3& operator/=(U other) noexcept;
               #pragma endregion
               #pragma region create new
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3 operator+(U other) const noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3 operator-(U other) const noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3 operator*(U other) const noexcept;
                  template<typename U> requires std::is_arithmetic_v<U> constexpr vec3 operator/(U other) const noexcept;
               #pragma endregion
            #pragma endregion
         #pragma endregion
   };
}

#include "./vec3.inl"