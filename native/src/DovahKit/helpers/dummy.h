#pragma once

namespace cobb {
   //
   // An empty no-op object with templated constructors, operator overloads, and similar which 
   // "eat" any arguments passed. Dummies of this type always compare as not equal to each other.
   //
   class dummy {
      public:
         dummy() {}
         template<typename... Args> requires (sizeof...(Args) > 0) dummy(Args&&...) {}

         template<typename T> dummy& operator=(const T&) { return *this; }
         template<typename T> dummy& operator=(T&&) noexcept { return *this; }

         template<typename T> bool operator==(const T&) const noexcept { return false; }
         template<typename T> bool operator!=(const T&) const noexcept { return true; }

         template<typename T> dummy operator+(const T&) const { return {}; }
         template<typename T> dummy operator-(const T&) const { return {}; }
         template<typename T> dummy operator*(const T&) const { return {}; }
         template<typename T> dummy operator/(const T&) const { return {}; }
         template<typename T> dummy operator%(const T&) const { return {}; }
         template<typename T> dummy operator&(const T&) const { return {}; }
         template<typename T> dummy operator|(const T&) const { return {}; }
         template<typename T> dummy operator^(const T&) const { return {}; }
         template<typename T> dummy operator<<(const T&) const { return {}; }
         template<typename T> dummy operator>>(const T&) const { return {}; }
         template<typename T> dummy& operator+=(const T&) { return *this; }
         template<typename T> dummy& operator-=(const T&) { return *this; }
         template<typename T> dummy& operator*=(const T&) { return *this; }
         template<typename T> dummy& operator/=(const T&) { return *this; }
         template<typename T> dummy& operator%=(const T&) { return *this; }
         template<typename T> dummy& operator&=(const T&) { return *this; }
         template<typename T> dummy& operator|=(const T&) { return *this; }
         template<typename T> dummy& operator^=(const T&) { return *this; }
         template<typename T> dummy& operator<<=(const T&) { return *this; }
         template<typename T> dummy& operator>>=(const T&) { return *this; }

         dummy operator+() const { return {}; }
         dummy operator-() const { return {}; }
         dummy operator!() const { return {}; }
   };
}