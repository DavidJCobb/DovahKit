#pragma once
#include <math.h>
#include <type_traits>

namespace cobb {
   template<typename T> class vector3 {
      public:
         static_assert(std::is_arithmetic_v<T>, "cobb::vector3 must be templated on a numeric type.");
         //
         T x = T(0);
         T y = T(0);
         T z = T(0);
         //
         vector3 cross(const vector3& other) const noexcept {
            vector3 result;
            result.x = (float)y * (float)other.z - (float)z * (float)other.y;
            result.y = (float)z * (float)other.x - (float)x * (float)other.z;
            result.z = (float)x * (float)other.y - (float)y * (float)other.x;
            return result;
         }
         float length_sq() const noexcept {
            float a = x * x;
            float b = y * y;
            float c = z * z;
            return a + b + c;
         }
         inline float length() const noexcept {
            return sqrtf(this->length_sq());
         }
         void normalize() noexcept {
            *this /= this->length();
         }
         //
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
         vector3& operator*=(const int& other) noexcept {
            x *= other;
            y *= other;
            z *= other;
            return *this;
         }
         vector3& operator/=(const int& other) noexcept {
            x /= other;
            y /= other;
            z /= other;
            return *this;
         }
         vector3  operator-() const noexcept {
            vector3 result = *this;
            result *= -1;
            return result;
         }
         vector3& operator+(const vector3& other) const noexcept {
            vector3 result = *this;
            result += other;
            return result;
         }
         vector3& operator-(const vector3& other) const noexcept {
            vector3 result = *this;
            result -= other;
            return result;
         }
   };

};