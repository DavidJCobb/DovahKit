#pragma once
#include <array>
#include <compare>
#include <memory>
#include <type_traits>

namespace cobb {
   template<typename T, size_t LocalCapacity, bool Contiguous = false>
   class small_vector {
      public:
         using value_type = T;
         using size_type  = size_t;
         static constexpr const size_type local_capacity = LocalCapacity;
         static constexpr const bool contiguous = Contiguous;

         using reference = value_type&;
         using const_reference = const value_type&;

      protected:
         static constexpr const bool _default_insertable = std::is_default_constructible_v<value_type>;
         static constexpr const bool _copy_insertable = std::is_copy_assignable_v<value_type> || std::is_copy_constructible_v<value_type>;
         static constexpr const bool _move_insertable = std::is_move_assignable_v<value_type> || std::is_move_constructible_v<value_type>;

         struct dummy {};

         template<typename V>
         static constexpr const bool _can_construct_local_range = ((_default_insertable && std::is_same_v<V, dummy>) || std::is_same_v<V, const value_type&>);

         constexpr void _construct(value_type* dst, value_type& src) requires(_copy_insertable || _move_insertable);
         constexpr void _transfer(value_type& dst, value_type& src) requires(_copy_insertable || _move_insertable);

         template<typename V = dummy>
         constexpr void _construct_local_range(size_type begin, size_type end, V v) requires (_can_construct_local_range<V>);

         template<typename V = dummy>
         constexpr void _resize_impl(size_type s, V v) requires (_can_construct_local_range<V>);

      protected:
         struct {
            union _ {
               std::array<value_type, LocalCapacity> list;
               bool _this_union_and_bool_prevent_local_auto_construct_destruct = {};

               ~_() {}
            } local;
            value_type* heap = nullptr;
         } _data;
         size_t _size = 0;

      #pragma region Iterator definitions
      protected:
         template<bool Const> struct _contiguous_iterator {
            using list_type   = std::conditional_t<Const, const small_vector, small_vector>;
            using target_type = std::conditional_t<Const, const value_type, value_type>;

            target_type* target = nullptr;

            constexpr _contiguous_iterator(list_type& o) {
               if (o.empty())
                  return;
               target = &o[0];
            }
            constexpr _contiguous_iterator(list_type& o, size_t i) {
               if (o.empty())
                  return;
               target = &o[i];
            }
            constexpr _contiguous_iterator(_contiguous_iterator& o) : target(o.target) {}

            target_type& operator*() const { return *target; }
            target_type* operator->() const { return target; }

            constexpr _contiguous_iterator& operator++() { return operator+=(1); }
            constexpr _contiguous_iterator& operator--() { return operator-=(1); }
            constexpr _contiguous_iterator  operator++(int) { return _contiguous_iterator(*this) += 1; }
            constexpr _contiguous_iterator  operator--(int) { return _contiguous_iterator(*this) -= 1; }
            constexpr _contiguous_iterator& operator+=(size_t i) { target += i; return *this; }
            constexpr _contiguous_iterator& operator-=(size_t i) { target -= i; return *this; }

            friend constexpr std::partial_ordering operator<=>(const _contiguous_iterator& a, const _contiguous_iterator& b) {
               return a.target <=> b.target;
            }
            constexpr bool operator==(const _contiguous_iterator& other) const {
               return (*this <=> other) == std::partial_ordering::equivalent;
            }
         };
         template<bool Const> struct _split_iterator {
            using list_type   = std::conditional_t<Const, const small_vector, small_vector>;
            using target_type = std::conditional_t<Const, const value_type, value_type>;

            list_type& owner;
            size_t     index = 0;

            constexpr _split_iterator(list_type& o) : owner(o) {}
            constexpr _split_iterator(list_type& o, size_t i) : owner(o), index(i) {}
            constexpr _split_iterator(_split_iterator& o) : owner(o.owner), index(o.index) {}

            target_type& operator*() const { return owner[index]; }
            target_type* operator->() const { return &owner[index]; }

            constexpr _split_iterator& operator++() { return operator+=(1); }
            constexpr _split_iterator& operator--() { return operator-=(1); }
            constexpr _split_iterator  operator++(int) { return _split_iterator(*this) += 1; }
            constexpr _split_iterator  operator--(int) { return _split_iterator(*this) -= 1; }
            constexpr _split_iterator& operator+=(size_t i) { index += i; return *this; }
            constexpr _split_iterator& operator-=(size_t i) { index -= i; return *this; }

            friend constexpr std::partial_ordering operator<=>(const _split_iterator& a, const _split_iterator& b) {
               if (&a.owner != &b.owner) {
                  return std::partial_ordering::unordered;
               }
               return a.index <=> b.index;
            }
            constexpr bool operator==(const _split_iterator& other) const {
               return (*this <=> other) == std::partial_ordering::equivalent;
            }
         };

         template<bool Const> using _actual_iterator = std::conditional_t<Contiguous, _contiguous_iterator<Const>, _split_iterator<Const>>;
      #pragma endregion

      public:
         using iterator = _actual_iterator<false>;
         using const_iterator = _actual_iterator<true>;
         
      public:
         constexpr small_vector() {}
         constexpr ~small_vector() {
            clear();
         }
            
         constexpr const_reference operator[](size_type i) const {
            if constexpr (contiguous) {
               if (_size > local_capacity)
                  return _data.heap[i];
            } else {
               if (i >= local_capacity)
                  return _data.heap[i - local_capacity];
            }
            return _data.local.list[i];
         }
         constexpr reference operator[](size_type i) {
            return const_cast<reference>(const_cast<const small_vector*>(this)->operator[](i));
         }

         constexpr bool empty() const noexcept { return _size == 0; }
         constexpr size_type size() const noexcept { return _size; }

         constexpr const value_type* data() const noexcept requires contiguous {
            if (_size > local_capacity)
               return _data.heap;
            return _data.local.list.data();
         }

         constexpr void resize(size_type s) requires (_default_insertable) {
            _resize_impl(s, dummy{});
         }
         constexpr void resize(size_type s, const value_type& v) requires (_copy_insertable) {
            _resize_impl(s, v);
         }

         constexpr void clear() {
            if constexpr (contiguous) {
               if (_size <= local_capacity) {
                  std::destroy(_data.local.list.begin(), _data.local.list.begin() + _size);
               }
            } else {
               std::destroy(_data.local.list.begin(), _data.local.list.begin() + _size);
            }
            _size = 0;
            if (_data.heap) {
               delete[] _data.heap;
               _data.heap = nullptr;
            }
         }

         template<typename... Args>
         reference emplace_back(Args&&... args) {
            auto i = _size;
            resize(_size + 1);
            //
            auto& item = operator[](i);
            std::construct_at(&item, std::forward<Args>(args)...);
            return item;
         }

         void push_back(const value_type& v) {
            auto i = _size;
            resize(_size + 1);
            operator[](i) = v;
         }
         void push_back(value_type&& v) {
            auto i = _size;
            resize(_size + 1);
            operator[](i) = std::move(v);
         }

         iterator begin() { return iterator(*this, 0); }
         iterator end() { return iterator(*this, size()); }
         const_iterator cbegin() const { return const_iterator(*this, 0); }
         const_iterator cend() const { return const_iterator(*this, size()); }
         const_iterator begin() const { return const_iterator(*this, 0); }
         const_iterator end() const { return const_iterator(*this, size()); }
   };
}

#include "small_vector.inl"