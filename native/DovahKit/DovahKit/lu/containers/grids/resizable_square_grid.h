#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

namespace lu {
   namespace impl::_resizable_grid {
      struct dummy {};

      struct no_op_functor {
         template<typename T> void operator()(T&) {}
      };
   }

   //
   // A resizable square grid centered on (0, 0). If the grid length is an even number, 
   // then the grid biases toward the upper-left (i.e. the center is closer to the lower-
   // right corner).
   //
   template<typename T, typename CoordinateType = int32_t> requires std::is_signed_v<CoordinateType>
   class resizable_square_grid {
      public:
         using value_type      = T;
         using coordinate_type = CoordinateType;
         using length_type     = std::make_unsigned_t<CoordinateType>;
         using size_type       = std::size_t;
         
      protected:
         static constexpr const bool _can_default = std::is_default_constructible_v<value_type>;
         static constexpr const bool _can_copy    = std::is_copy_constructible_v<value_type> || std::is_copy_assignable_v<value_type>;
         static constexpr const bool _can_move    = std::is_move_constructible_v<value_type> || std::is_move_assignable_v<value_type>;

         struct _no_op_destroy_functor {
            void operator()(value_type&) {}
         };
         
      protected:
         template<bool Const> class _base_iterator {
            friend resizable_square_grid;
            using  target_type = std::conditional_t<Const, std::add_const_t<value_type>, std::remove_const_t<value_type>>;
            protected:
               target_type* target = nullptr;

               _base_iterator() {}
               _base_iterator(target_type* v) : target(v) {}

            public:
               target_type& operator*() { return *target; }
               target_type& operator*() const { return *target; }

               bool operator==(const _base_iterator&) const noexcept = default;

               _base_iterator& operator++() { ++target; return *this; }
               _base_iterator& operator--() { --target; return *this; }
               _base_iterator& operator+=(size_t n) { target += n; return *this; }
               _base_iterator& operator-=(size_t n) { target -= n; return *this; }

               _base_iterator operator++(int) const { return _base_iterator(*this)++; }
               _base_iterator operator--(int) const { return _base_iterator(*this)--; }
               _base_iterator operator+(size_t n) const { return _base_iterator(*this) += n; }
               _base_iterator operator-(size_t n) const { return _base_iterator(*this) -= n; }
         };
      public:
         using iterator = _base_iterator<false>;
         using const_iterator = _base_iterator<true>;

      protected:
         value_type* _data = nullptr;
         length_type _length = 0;

         #pragma region Internal alloc/free
            #ifdef _MSC_VER
               __declspec(allocator)
            #endif
            static constexpr value_type* _alloc(size_t count, bool skip_construct = false);

            static constexpr void _free(value_type* memory, size_t count, bool skip_destruct = false);
         #pragma endregion

         template<typename Functor>
         static constexpr void _for_perimeter(value_type* buffer, length_type outer_length, length_type inner_length, Functor&& functor);

         static constexpr value_type& _item_in_new_grid(value_type* buffer, length_type length, length_type x, length_type y) {
            return buffer[x + (y * length)];
         }
         
         template<typename InitializeNewTo = impl::_resizable_grid::dummy, typename DestroyFunctor = impl::_resizable_grid::dummy>
         constexpr void _resize_impl(length_type new_length, const InitializeNewTo& values, DestroyFunctor destroy_functor = {});

      public:
         #pragma region Constructor, destructor, and assignment operators
            constexpr resizable_square_grid() {}

            template<typename U> requires (std::is_convertible_v<U, length_type> || std::is_same_v<U, length_type>) constexpr resizable_square_grid(U l) {
               this->resize((length_type)l);
            }

            constexpr resizable_square_grid(const resizable_square_grid& v) {
               *this = v;
            }
            constexpr resizable_square_grid(resizable_square_grid&& v) {
               *this = std::move(v);
            }

            constexpr ~resizable_square_grid() {
               this->clear_and_collapse();
            }

            constexpr resizable_square_grid& operator=(const resizable_square_grid& o) requires (_can_copy);
            constexpr resizable_square_grid& operator=(resizable_square_grid&& o) {
               std::swap(this->_data,   o._data);
               std::swap(this->_length, o._length);
               return *this;
            }
         #pragma endregion

         constexpr size_type area() const noexcept { return (size_type)length() * (size_type)length(); }
         constexpr const value_type* data() const noexcept { return this->_data; }
         constexpr value_type* data() noexcept { return this->_data; }
         constexpr bool empty() const noexcept { return this->length() != 0; }
         constexpr length_type length() const noexcept { return _length; }

         #pragma region Edge coordinates
            // X-coordinate of the leftmost grid items.
            constexpr coordinate_type left() const noexcept { return -(this->length() / 2); }

            // X-coordinate of the rightmost grid items (i.e. the coordinate is inside the grid, not past its edge).
            constexpr coordinate_type right() const noexcept { return (this->length() / 2) - (this->length() % 2 ? 0 : 1); }

            // Y-coordinate of the topmost grid items.
            constexpr coordinate_type top() const noexcept { return left(); }

            // Y-coordinate of the bottommost grid items (i.e. the coordinate is inside the grid, not past its edge).
            constexpr coordinate_type bottom() const noexcept { return right(); }
         #pragma endregion

         #pragma region Iterator-based access
            iterator begin() { return iterator{ _data }; }
            iterator end() { return iterator{ _data + area() }; }
            const_iterator cbegin() const { return const_iterator{ _data }; }
            const_iterator cend() const { return const_iterator{ _data + area() }; }
            const_iterator begin() const { return cbegin(); }
            const_iterator end() const { return cend(); }
         #pragma endregion

      public:
         // Reset all grid items to their default values.
         constexpr void clear() requires (_can_default);

         // Set the grid's size to zero.
         constexpr void clear_and_collapse();

         constexpr bool contains_coordinate(coordinate_type x, coordinate_type y) const noexcept;

         #pragma region for_each
            template<typename Functor>
            void for_each(Functor&& functor) const
               requires (
                  std::is_invocable_v<Functor, const value_type&, coordinate_type, coordinate_type> ||
                  std::is_invocable_v<Functor, const value_type&>
               )
            ;
            
            template<typename Functor>
            void for_each(Functor&& functor)
               requires (
                  std::is_invocable_v<Functor, value_type&, coordinate_type, coordinate_type> ||
                  std::is_invocable_v<Functor, value_type&>
               )
            ;
         #pragma endregion

         #pragma region item
            constexpr const value_type& item(coordinate_type x, coordinate_type y) const noexcept;
            constexpr value_type& item(coordinate_type x, coordinate_type y) noexcept;
         #pragma endregion

         #pragma region item_by_flat_index
            // Access an item by its row-major index, with the top-left item being index 0.
            constexpr const value_type& item_by_flat_index(size_t) const noexcept;

            // Access an item by its row-major index, with the top-left item being index 0.
            constexpr value_type& item_by_flat_index(size_t) noexcept;
         #pragma endregion

         #pragma region item_from_corner
            // Access an item given grid coordinates measured from the top-left (i.e. negative) corner.
            constexpr const value_type& item_from_corner(length_type x, length_type y) const noexcept;

            // Access an item given grid coordinates measured from the top-left (i.e. negative) corner.
            constexpr value_type& item_from_corner(length_type x, length_type y) noexcept;
         #pragma endregion

         #pragma region resize
            constexpr void resize(length_type new_length) requires (_can_default || _can_move);

            template<typename DestroyFunctor = _no_op_destroy_functor>
            constexpr void resize(length_type new_length, const DestroyFunctor& destroy = {})
               requires (
                  (_can_default || _can_move) &&
                  !std::is_same_v<DestroyFunctor, value_type> &&
                  std::is_invocable_v<DestroyFunctor, value_type&>
               )
            ;

            template<typename DestroyFunctor = _no_op_destroy_functor>
            constexpr void resize(length_type new_length, const value_type& v, const DestroyFunctor& destroy = {})
               requires (
                  (_can_copy && _can_move) &&
                  std::is_invocable_v<DestroyFunctor, value_type&>
               )
            ;
         #pragma endregion

         // Shift all grid items by a fixed offset. Items that are shifted outside of the grid's 
         // bounds are passed to the provided `destroy_functor`.
         template<typename Functor>
         constexpr void shift_by(coordinate_type x_delta, coordinate_type y_delta, Functor&& destroy_functor);
   };
}

#include "./resizable_square_grid.inl"