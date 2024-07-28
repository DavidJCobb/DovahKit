#pragma once
#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include "../data/sex.h"

namespace dovah {
   //
   // Store data by sex, allowing access using the enum or a numeric index. 
   // You can also iterate over the data, accessing the male data first and 
   // then the female data. (This matches the ordering of the game's own sex 
   // enum.)
   //
   template<typename T>
   class data_by_sex {
      protected:
         template<bool Const>
         class _iterator {
            using list_type   = std::conditional_t<Const, const T, T>;
            using target_type = std::conditional_t<Const, const T, T>;
            
            target_type* target;

            public:
               constexpr _iterator(target_type* o) : target(o) {}

               T& operator*() const { return *target; }
               T* operator->() const { return target; }

               constexpr _iterator& operator++() { return operator+=(1); }
               constexpr _iterator& operator--() { return operator-=(1); }
               constexpr _iterator  operator++(int) { return _iterator(*this) += 1; }
               constexpr _iterator  operator--(int) { return _iterator(*this) -= 1; }
               constexpr _iterator& operator+=(size_t i) { target += i; return *this; }
               constexpr _iterator& operator-=(size_t i) { target -= i; return *this; }

               friend constexpr std::partial_ordering operator<=>(const _iterator& a, const _iterator& b) {
                  return a.target <=> b.target;
               }
               constexpr bool operator==(const _iterator& other) const {
                  return (*this <=> other) == std::partial_ordering::equivalent;
               }
         };
      public:
         using iterator = _iterator<false>;
         using const_iterator = _iterator<true>;

      public:
         T male   = {};
         T female = {};

         constexpr const T& operator[](sex i) const {
            switch (i) {
               case sex::male:
                  return this->male;
               case sex::female:
                  return this->female;
            }
            throw std::out_of_range("invalid sex");
         }
         constexpr const T& operator[](size_t i) const {
            return operator[]((sex)i);
         }

         constexpr T& operator[](sex i) {
            return const_cast<T&>(std::as_const(*this)[i]);
         }
         constexpr T& operator[](size_t i) {
            return const_cast<T&>(std::as_const(*this)[i]);
         }

         constexpr size_t size() const noexcept {
            return sex_count;
         }

         iterator begin() { return iterator(&this->male); }
         iterator end()   { return iterator(&this->female + 1); }
         const_iterator begin() const { return const_iterator(&this->male); }
         const_iterator end()   const { return const_iterator(&this->female + 1); }

         const_iterator cbegin() const { return const_iterator(&this->male); }
         const_iterator cend()   const { return const_iterator(&this->female + 1); }
   };
}