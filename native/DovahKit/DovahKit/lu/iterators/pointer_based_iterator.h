#pragma once
#include <iterator>
#include <type_traits>

namespace lu::iterators {
   template<typename Container, typename ValueType, bool Const>
   class pointer_based_iterator {
      friend Container;
      template<typename Container, typename ValueType, bool Const> friend class pointer_based_iterator;
      public:
         using value_type = ValueType;
         using pointer    = std::conditional_t<Const, const value_type*, value_type*>;
         using reference  = std::conditional_t<Const, const value_type&, value_type&>;

         using difference_type   = std::ptrdiff_t;
         using iterator_category = std::contiguous_iterator_tag;

      protected:
         template<bool C>
         using _with_constness = pointer_based_iterator<Container, ValueType, C>;

      protected:
         constexpr pointer_based_iterator() noexcept {}
         constexpr pointer_based_iterator(pointer o) noexcept {
            this->target = o;
         }

      public:
         constexpr pointer_based_iterator(const pointer_based_iterator& o) noexcept {
            this->target = o.target;
         }
         constexpr pointer_based_iterator(const _with_constness<false>& o) noexcept requires (Const) {
            this->target = o.target;
         }

      protected:
         pointer target = nullptr;
                     
      public:
         constexpr reference operator*() const {
            return *this->target;
         }
         constexpr pointer operator->() const {
            return *this->target;
         }
         
         constexpr pointer_based_iterator& operator+=(difference_type i) { this->target += i; return *this; }
         constexpr pointer_based_iterator& operator-=(difference_type i) { this->target -= i; return *this; }
         constexpr pointer_based_iterator  operator+(difference_type i) { auto v = *this; v += i; return v; }
         constexpr pointer_based_iterator  operator-(difference_type i) { auto v = *this; v -= i; return v; }
         constexpr pointer_based_iterator& operator++() { return operator+=(1); }
         constexpr pointer_based_iterator& operator--() { return operator-=(1); }
         constexpr pointer_based_iterator  operator++(int) { auto v = *this; operator+=(); return v; }
         constexpr pointer_based_iterator  operator--(int) { auto v = *this; operator-=(); return v; }

         friend constexpr pointer_based_iterator operator+(const difference_type o, pointer_based_iterator it) noexcept {
            it += o;
            return it;
         }
         
         constexpr difference_type operator-(const pointer_based_iterator& o) {
            return this->target - o.target;
         }
         
         template<bool OtherConst>
         constexpr std::strong_ordering operator<=>(const _with_constness<OtherConst>& other) const noexcept {
            return this->target <=> other.target;
         }
         template<bool OtherConst>
         constexpr bool operator==(const _with_constness<OtherConst>& other) const noexcept {
            return this->target == other.target;
         }
         template<bool OtherConst>
         constexpr bool operator!=(const _with_constness<OtherConst>& other) const noexcept {
            return this->target != other.target;
         }
   };
}
