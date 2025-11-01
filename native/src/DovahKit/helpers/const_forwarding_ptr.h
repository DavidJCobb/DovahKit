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
   class const_forwarding_ptr {
      private:
         T* target;

         struct _static_warning {
            constexpr _static_warning() requires (!std::is_const_v<T>) {}
            [[deprecated("Using a `const_forwarding_ptr` for a pointer that is always const is redundant.")]] constexpr _static_warning() requires (std::is_const_v<T>) {}
         };
         [[maybe_unused]] [[no_unique_address]] _static_warning _issue_static_warning{};

      public:
         constexpr const_forwarding_ptr() : target(nullptr) {}
         constexpr const_forwarding_ptr(std::nullptr_t) : target(nullptr) {}

         template<typename U> requires std::is_base_of_v<T, U>
         constexpr const_forwarding_ptr(U* v) : target(v) {}

         template<typename U> requires std::is_base_of_v<T, U>
         constexpr const_forwarding_ptr& operator=(U* v) {
            this->target = v;
            return *this;
         }

         // Conversion.
         constexpr operator T*() noexcept { return this->target; }
         constexpr operator const T*() const noexcept { return this->target; }
         
         // Pointer arithmetic.
         constexpr const_forwarding_ptr& operator++() { ++this->target; return *this; }
         constexpr const_forwarding_ptr operator++(int) const noexcept { auto copy = *this; ++copy; return copy; }
         constexpr const_forwarding_ptr& operator--() { --this->target; return *this; }
         constexpr const_forwarding_ptr operator--(int) const noexcept { auto copy = *this; --copy; return copy; }

         // Dereferencing.
         constexpr T& operator*() noexcept { return *this->target; }
         constexpr const T& operator*() const noexcept { return *this->target; }
         constexpr T& operator[](size_t i) noexcept { return this->target[i]; }
         constexpr const T& operator[](size_t i) const noexcept { return this->target[i]; }
         //
         constexpr T* operator->() noexcept { return this->target; }
         constexpr const T* operator->() const noexcept { return this->target; }
   };
   static_assert(sizeof(const_forwarding_ptr<void>) == sizeof(void*)); // verify that our static warning hack doesn't affect size
}