#pragma once
#include <compare>
#include <type_traits>

namespace cobb {
   namespace impl {
      template<typename Iterator>
      class const_pointer_container_iterator {
         public:
            using underlying_value_type = std::decay_t<decltype(*(Iterator{}))>;
            using pointed_to_value_type = std::remove_pointer_t<underlying_value_type>;

         public:
            using value_type = std::add_pointer_t<std::add_const_t<pointed_to_value_type>>;

         protected:
            Iterator underlying;

         public:
            constexpr const_pointer_container_iterator(Iterator i) : underlying(i) {}

            constexpr value_type  operator*() const { return *underlying; }
            constexpr value_type* operator->() const { return underlying; }

            constexpr const_pointer_container_iterator& operator++() { return operator+=(1); }
            constexpr const_pointer_container_iterator& operator--() { return operator-=(1); }
            constexpr const_pointer_container_iterator  operator++(int) { return const_pointer_container_iterator(*this) += 1; }
            constexpr const_pointer_container_iterator  operator--(int) { return const_pointer_container_iterator(*this) -= 1; }
            constexpr const_pointer_container_iterator& operator+=(size_t i) { underlying += i; return *this; }
            constexpr const_pointer_container_iterator& operator-=(size_t i) { underlying -= i; return *this; }

            friend constexpr std::partial_ordering operator<=>(const const_pointer_container_iterator& a, const const_pointer_container_iterator& b) {
               return a.underlying <=> b.underlying;
            }
            constexpr bool operator==(const const_pointer_container_iterator& other) const {
               return (*this <=> other) == std::partial_ordering::equivalent;
            }
      };

      template<typename Container>
      concept const_pointer_container_viewable = requires (const Container& c) {
         typename Container::value_type;
         typename Container::iterator;
         requires std::is_pointer_v<typename Container::value_type>;
      };
   }

   template<impl::const_pointer_container_viewable Container>
   class const_pointer_container_view {
      public:
         using underlying_value_type = typename Container::value_type;
         using pointed_to_value_type = std::remove_pointer_t<underlying_value_type>;

      public:
         using value_type = std::add_pointer_t<std::add_const_t<pointed_to_value_type>>;
         using iterator   = impl::const_pointer_container_iterator<typename Container::const_iterator>;
         using const_iterator = iterator;

      protected:
         const Container& target;

      public:
         constexpr const_pointer_container_view(const Container& c) : target(c) {}

         constexpr iterator begin() const noexcept {
            return iterator(target.cbegin());
         }
         constexpr iterator end() const noexcept {
            return iterator(target.cend());
         }
         constexpr iterator cbegin() const noexcept { return begin(); }
         constexpr iterator cend() const noexcept { return end(); }

         constexpr value_type operator[](size_t at) const noexcept {
            return target[at];
         }
   };
}