#pragma once
#include <vector>
namespace dovah {
   class form_stub;
}

namespace dovah::form_data::pieces {
   //
   // A wrapper around `std::vector` for use with value types that either are managed form/string 
   // uses, or contain managed form/string uses. Mutating accessors require passing a reference 
   // to the `form_stub` that owns the vector's containing form-data, so that use info can be 
   // updated properly.
   //
   template<typename T>
   class managed_vector {
      public:
         using wrapped_container_type = std::vector<T>;

         using value_type      = wrapped_container_type::value_type;
         using size_type       = wrapped_container_type::size_type;
         using iterator        = wrapped_container_type::iterator;
         using const_iterator  = wrapped_container_type::const_iterator;
         using reference       = wrapped_container_type::reference;
         using const_reference = wrapped_container_type::const_reference;
         using pointer         = wrapped_container_type::pointer;
         using const_pointer   = wrapped_container_type::pointer;
         
      protected:
         wrapped_container_type _wrapped;

      public:
         constexpr managed_vector() {}
         #pragma region Move
            managed_vector(managed_vector&&) noexcept = default;
            managed_vector& operator=(managed_vector&&) noexcept = default;
         #pragma endregion
         #pragma region Copy
            managed_vector(const managed_vector&) = delete;
            managed_vector& operator=(const managed_vector&) = delete;

            constexpr void assign(form_stub& containing_form, const managed_vector&);
         #pragma endregion

         constexpr const size_type capacity() const noexcept { return this->_wrapped.capacity(); }
         constexpr const bool empty() const noexcept { return this->_wrapped.empty(); }
         constexpr const size_type max_size() const noexcept { return this->_wrapped.max_size(); }
         constexpr const size_type size() const noexcept { return this->_wrapped.size(); }

         constexpr void reserve(size_type s) { this->reserve(s); }
         constexpr void shrink_to_fit() { this->shrink_to_fit(); }

         // Use when loading form data.
         constexpr void unmanaged_push_back(T&& v) { this->_wrapped.push_back(v); }

         #pragma region Element access
            const_reference at(size_type s) const noexcept { return this->_wrapped.at(s); }
            reference at(size_type s) noexcept { return this->_wrapped.at(s); }
            constexpr const_reference operator[](size_type s) const noexcept { return this->_wrapped[s]; }
            constexpr reference operator[](size_type s) noexcept { return this->_wrapped[s]; }
            constexpr const_reference front() const noexcept { return this->_wrapped.front(); }
            constexpr reference front() noexcept { return this->_wrapped.front(); }
            constexpr const_reference back() const noexcept { return this->_wrapped.back(); }
            constexpr reference back() noexcept { return this->_wrapped.back(); }
            constexpr const_pointer data() const noexcept { return this->_wrapped.data(); }
            constexpr pointer data() noexcept { return this->_wrapped.data(); }
         #pragma endregion
         #pragma region Iterators
            constexpr const_iterator begin() const noexcept { return this->_wrapped.cbegin(); }
            constexpr const_iterator cbegin() const noexcept { return this->_wrapped.cbegin(); }
            constexpr iterator begin() noexcept { return this->_wrapped.begin(); }
            constexpr const_iterator end() const noexcept { return this->_wrapped.cend(); }
            constexpr const_iterator cend() const noexcept { return this->_wrapped.cend(); }
            constexpr iterator end() noexcept { return this->_wrapped.end(); }
            constexpr const_iterator rbegin() const noexcept { return this->_wrapped.crbegin(); }
            constexpr const_iterator crbegin() const noexcept { return this->_wrapped.crbegin(); }
            constexpr iterator rbegin() noexcept { return this->_wrapped.rbegin(); }
            constexpr const_iterator rend() const noexcept { return this->_wrapped.crend(); }
            constexpr const_iterator crend() const noexcept { return this->_wrapped.crend(); }
            constexpr iterator rend() noexcept { return this->_wrapped.rend(); }
         #pragma endregion
         #pragma region Modifiers
            constexpr void clear(form_stub& containing_form);
            constexpr iterator erase(form_stub& containing_form, iterator);
            constexpr iterator erase(form_stub& containing_form, iterator first, iterator last);
            constexpr void pop_back(form_stub& containing_form);
            constexpr void resize(form_stub& containing_form, size_type);
         #pragma endregion

         //
         // Non-STL APIs:
         //
         value_type& append();
   };
}

#include "./managed_vector.inl"