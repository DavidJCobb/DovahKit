#pragma once
#include <type_traits>

namespace cobb {
   template<typename T> struct int_wrapper {
      protected:
         typedef T underlying_type;
         T value;
      public:
         int_wrapper() noexcept : value() {};
         constexpr int_wrapper(const T& v) noexcept : value(v) {};

         template<typename U, std::enable_if_t<std::is_integral_v<U>>> operator U() const noexcept { return (U)this->value; }
         template<typename U, std::enable_if_t<std::is_integral_v<U>>> constexpr operator U() const noexcept { return (U)this->value; }
         operator T&() noexcept { return this->value; }
         operator const T&() const noexcept { return this->value; }
         //
         int_wrapper& operator= (T other) { this->value = other; return *this; }
         int_wrapper& operator+=(T other) { this->value += other; return *this; }
         int_wrapper& operator-=(T other) { this->value -= other; return *this; }
         int_wrapper& operator*=(T other) { this->value *= other; return *this; }
         int_wrapper& operator/=(T other) { this->value /= other; return *this; }
         int_wrapper& operator%=(T other) { this->value %= other; return *this; }
         //
         int_wrapper& operator++() { ++this->value; return *this; }
         int_wrapper& operator--() { --this->value; return *this; }
         int_wrapper  operator++(int) { return int_wrapper(this->value++); }
         int_wrapper  operator--(int) { return int_wrapper(this->value--); }
         //
         int_wrapper& operator&=(T other) { this->value &= other; return *this; }
         int_wrapper& operator|=(T other) { this->value |= other; return *this; }
         int_wrapper& operator^=(T other) { this->value ^= other; return *this; }
         int_wrapper& operator<<=(T other) { this->value <<= other; return *this; }
         int_wrapper& operator>>=(T other) { this->value >>= other; return *this; }
         //
         int_wrapper operator+() const { return int_wrapper(+this->value); }
         int_wrapper operator-() const { return int_wrapper(-this->value); }
         int_wrapper operator!() const { return int_wrapper(!this->value); }
         int_wrapper operator~() const { return int_wrapper(~this->value); }
         //
         friend int_wrapper operator+(int_wrapper me, int_wrapper other) { return me += other; }
         friend int_wrapper operator+(int_wrapper me, T other) { return me += other; }
         friend int_wrapper operator+(T other, int_wrapper me) { return int_wrapper(v) += me; }
         friend int_wrapper operator-(int_wrapper me, int_wrapper other) { return me -= other; }
         friend int_wrapper operator-(int_wrapper me, T other) { return me -= other; }
         friend int_wrapper operator-(T other, int_wrapper me) { return int_wrapper(v) -= me; }
         friend int_wrapper operator*(int_wrapper me, int_wrapper other) { return me *= other; }
         friend int_wrapper operator*(int_wrapper me, T other) { return me *= other; }
         friend int_wrapper operator*(T other, int_wrapper me) { return int_wrapper(v) *= me; }
         friend int_wrapper operator/(int_wrapper me, int_wrapper other) { return me /= other; }
         friend int_wrapper operator/(int_wrapper me, T other) { return me /= other; }
         friend int_wrapper operator/(T other, int_wrapper me) { return int_wrapper(v) /= me; }
         friend int_wrapper operator%(int_wrapper me, int_wrapper other) { return me %= other; }
         friend int_wrapper operator%(int_wrapper me, T other) { return me %= other; }
         friend int_wrapper operator%(T other, int_wrapper me) { return int_wrapper(v) %= me; }
         friend int_wrapper operator&(int_wrapper me, int_wrapper other) { return me &= other; }
         friend int_wrapper operator&(int_wrapper me, T other) { return me &= other; }
         friend int_wrapper operator&(T other, int_wrapper me) { return int_wrapper(v) &= me; }
         friend int_wrapper operator|(int_wrapper me, int_wrapper other) { return me |= other; }
         friend int_wrapper operator|(int_wrapper me, T other) { return me |= other; }
         friend int_wrapper operator|(T other, int_wrapper me) { return int_wrapper(v) |= me; }
         friend int_wrapper operator^(int_wrapper me, int_wrapper other) { return me ^= other; }
         friend int_wrapper operator^(int_wrapper me, T other) { return me ^= other; }
         friend int_wrapper operator^(T other, int_wrapper me) { return int_wrapper(v) ^= me; }
         friend int_wrapper operator<<(int_wrapper me, int_wrapper other) { return me <<= other; }
         friend int_wrapper operator<<(int_wrapper me, T other) { return me <<= other; }
         friend int_wrapper operator<<(T other, int_wrapper me) { return int_wrapper(v) <<= me; }
         friend int_wrapper operator>>(int_wrapper me, int_wrapper other) { return me >>= other; }
         friend int_wrapper operator>>(int_wrapper me, T other) { return me >>= other; }
         friend int_wrapper operator>>(T other, int_wrapper me) { return int_wrapper(v) >>= me; }
   };
}