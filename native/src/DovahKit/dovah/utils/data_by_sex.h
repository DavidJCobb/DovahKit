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
         class iterator {
            using list_type   = std::conditional_t<Const, const T, T>;
            using target_type = std::conditional_t<Const, const T, T>;
            
            target_type* target;

            public:
               constexpr iterator(target_type* o) : dst(o) {}

               T& operator*() const { return *target; }
               T* operator->() const { return target; }

               constexpr iterator& operator++() { return operator+=(1); }
               constexpr iterator& operator--() { return operator-=(1); }
               constexpr iterator  operator++(int) { return iterator(*this) += 1; }
               constexpr iterator  operator--(int) { return iterator(*this) -= 1; }
               constexpr iterator& operator+=(size_t i) { target += i; return *this; }
               constexpr iterator& operator-=(size_t i) { target -= i; return *this; }

               constexpr std::partial_ordering operator<=>(const iterator& a, const iterator& b) {
                  return a.target <=> b.target;
               }
               constexpr bool operator==(const iterator& other) const {
                  return (*this <=> other) == std::partial_ordering::equivalent;
               }
         };

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

         iterator<false> begin() { return iterator(&this->male); }
         iterator<false> end()   { return iterator(&this->female + 1); }
         const iterator<true> begin() const { return iterator(&this->male); }
         const iterator<true> end()   const { return iterator(&this->female + 1); }

         const iterator<true> cbegin() const { return iterator(&this->male); }
         const iterator<true> cend()   const { return iterator(&this->female + 1); }
   };
}