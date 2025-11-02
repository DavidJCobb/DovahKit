#pragma once
#include <type_traits>

namespace cobb {
   /*
     
      A pointer that forwards its own constness to the pointed-to object. That 
      is: given a reference to the pointer, you gain non-const access to the 
      target; and given a const reference to the pointer, you gain const access 
      to the target. Essentially, it's either `T*` or `const T* const`, rather 
      than ever being just `T* const`.
     
   */
   template<typename T>
   class alignas(T*) const_forwarding_ptr {
      public:
         using value_type = T;
         using reference  = std::add_lvalue_reference_t<value_type>;

      private:
         value_type* target;

         struct _static_warning {
            constexpr _static_warning() requires (!std::is_const_v<value_type>) {}
            [[deprecated("Using a `const_forwarding_ptr` for a pointer that is always const is redundant.")]] constexpr _static_warning() requires (std::is_const_v<T>) {}
         };
         static constexpr const _static_warning _issue_static_warning{};

         static constexpr const bool _can_be_reference = !std::is_same_v<value_type, void>;

      public:
         constexpr const_forwarding_ptr() : target(nullptr) {}
         constexpr const_forwarding_ptr(std::nullptr_t) : target(nullptr) {}

         template<typename U> requires std::is_base_of_v<value_type, U>
         constexpr const_forwarding_ptr(U* v) : target(v) {}

         template<typename U> requires std::is_base_of_v<value_type, U>
         constexpr const_forwarding_ptr& operator=(U* v) {
            this->target = v;
            return *this;
         }

         // Conversion.
         constexpr operator value_type*() noexcept { return this->target; }
         constexpr operator const value_type*() const noexcept { return this->target; }
         
         // Pointer arithmetic.
         constexpr const_forwarding_ptr& operator++() { ++this->target; return *this; }
         constexpr const_forwarding_ptr operator++(int) const noexcept { auto copy = *this; ++copy; return copy; }
         constexpr const_forwarding_ptr& operator--() { --this->target; return *this; }
         constexpr const_forwarding_ptr operator--(int) const noexcept { auto copy = *this; --copy; return copy; }

         // Dereferencing.
         constexpr reference operator*() noexcept requires _can_be_reference { return *this->target; }
         constexpr const reference operator*() const noexcept requires _can_be_reference { return *this->target; }
         constexpr reference operator[](size_t i) noexcept requires _can_be_reference { return this->target[i]; }
         constexpr const reference operator[](size_t i) const noexcept requires _can_be_reference { return this->target[i]; }
         //
         constexpr value_type* operator->() noexcept { return this->target; }
         constexpr const value_type* operator->() const noexcept { return this->target; }
   };
   static_assert(sizeof(const_forwarding_ptr<void>) == sizeof(void*)); // verify that our static warning hack doesn't affect size
}