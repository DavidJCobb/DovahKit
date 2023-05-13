#pragma once
#include <type_traits>

namespace cobb::flat_tree_interface {
   template<typename Self, typename IndexType = size_t> // CRTP
   class node_base {
      public:
         using node       = Self;
         using index_type = IndexType;

      protected:
         index_type child_ahead = 0; // == index of first child - index of `this`, such that (this + this->first_child) == &the_first_child
         index_type child_count = 0;
         
      #pragma region Child list accessors
      protected:
         template<bool Const> class _child_range;

         template<bool Const> class _child_iterator_base {
            template<bool Const> friend class _child_range;
            friend class group;
            public:
               using iterator_category = std::forward_iterator_tag;
               using value_type        = std::conditional_t<Const, const node, node>;
               using pointer           = value_type*;
               using reference         = value_type&;
                     
            protected:
               pointer target = nullptr;

               constexpr _child_iterator_base(pointer t) : target(t) {}

            public:
               constexpr reference operator*() const { return *target; }
               constexpr pointer operator->() { return target; }

               constexpr _child_iterator_base& operator++() { ++target; return *this; }
               constexpr _child_iterator_base operator++(int) { _child_iterator_base out(*this); ++(*this); return out; }

               constexpr auto operator<=>(const _child_iterator_base&) const = default;
         };

         template<bool Const> class _child_range {
            friend class group;
            protected:
               using iterator       = _child_iterator_base<false>;
               using const_iterator = _child_iterator_base<true>;
               using target_type    = std::conditional_t<Const, const node, node>;
                     
               target_type& target;

               constexpr _child_range(target_type& t) : target(t) {}

               constexpr target_type* _first_child() const noexcept { return &target + target.child_ahead; }
               constexpr target_type* _after_last_child()  const noexcept { return _first_child() + target.child_count; }
               constexpr target_type* _last_child()  const noexcept { return _after_last_child() - 1; }

               constexpr void _require_children() const {
                  if (std::is_constant_evaluated()) {
                     if (!target.child_count)
                        throw;
                  }
               }

            public:
               constexpr iterator begin() { return iterator(_first_child()); }
               constexpr iterator end() { return iterator(_after_last_child()); }
               constexpr const_iterator cbegin() const { return const_iterator(_first_child()); }
               constexpr const_iterator cend() const { return const_iterator(_after_last_child()); }
               constexpr const_iterator begin() const { return cbegin(); }
               constexpr const_iterator end() const { return cend(); }

               constexpr size_t size() const noexcept {
                  return target.child_count;
               }
               constexpr bool empty() const noexcept {
                  return size() > 0;
               }

               constexpr const target_type& front() const { _require_children(); return *_first_child(); }
               constexpr target_type& front() requires (!Const) { _require_children(); return *_first_child(); }
               constexpr const target_type& back() const { _require_children(); return *_last_child(); }
               constexpr target_type& back() requires (!Const) { _require_children(); return *_last_child(); }

               constexpr const node& operator[](size_t i) const {
                  if (std::is_constant_evaluated()) {
                     if (i >= target.child_count)
                        throw;
                  }
                  return *(_first_child() + i);
               }
               constexpr node& operator[](size_t i) requires(!Const) {
                  return const_cast<node&>(std::as_const(*this).operator[](i));
               }
         };

      public:
         constexpr const _child_range<true> children() const noexcept { return _child_range<true>(*this); }
         constexpr _child_range<false> children() noexcept { return _child_range<false>(*this); }
      #pragma endregion
   };
}
