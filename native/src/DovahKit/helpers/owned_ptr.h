#pragma once
#include <type_traits>

namespace cobb {

   //
   // This class exists as an implicitly-casting alternative to std::unique_ptr. There 
   // are risks to implicit casting, most of which we are able to avoid; there are also 
   // ergonomic benefits.
   //
   // As such, cobb::owned_ptr is a minimal smart pointer with no lifetime management 
   // beyond deleting the pointed-to object in its destructor. It implicitly casts to 
   // a bare pointer, and disallows copy assignment and construction; the intended 
   // semantics are that bare pointers represent a lack of ownership.
   // 
   // 
   // == Risks of implicit casting, and mitigations ==
   // 
   // STL smart pointers do not implicitly cast to bare pointers, whereas cobb::owned_ptr 
   // does. There are risks to this, some of which we are able to avoid:
   // 
   //  - Math operations on implicitly-casting smart pointers become possible, and yield 
   //    bare pointers. This can be a problem when the math operation was intended to 
   //    target the pointed-to object (i.e. forgotten operator*).
   // 
   //     - We disable math operations via deleted operator overloads.
   // 
   //  - One implicitly-casting smart pointer can implicitly steal the bare pointer out 
   //    from another implicitly-casting smart pointer, such that one will eventually 
   //    delete the pointed-to object out from under the other.
   // 
   //     - Deleted copy-constructors and copy-assignments prevent this.
   // 
   //  - The delete operator becomes usable on implicitly-casting smart pointers, and 
   //    deletes the pointed-to object out from under the smart pointer.
   // 
   //     - If we disallow implicit conversion to `void*`, either directly or as a side 
   //       effect of how we implement implicit conversion, then it becomes impossible 
   //       to invoke operator delete(...) directly on our smart pointer as of this writing.
   // 
   // There are some benefits to implicit casting. Smart pointers must implicitly cast 
   // to bare pointers in order to allow certain comparisons, e.g. using std::find on a 
   // container of smart pointers given a bare pointer to search for.
   //


   // Deletes the pointed-to object on assignment to a new pointer, or on destruction.
   // No copy constructor allowed.
   template<typename T> class owned_ptr {
      public:
         using value_type = T;
         using pointer = value_type*;

      protected:
         pointer _target = nullptr;

      public:
         constexpr owned_ptr() {}
         constexpr owned_ptr(pointer v) : _target(v) {}
         //
         template<typename Derived> requires (std::is_base_of_v<value_type, Derived> && !std::is_same_v<Derived, value_type>)
         constexpr owned_ptr(Derived* b) : _target(b) {}

         template<typename U>
         owned_ptr(const owned_ptr<U>&) = delete;
         //
         constexpr owned_ptr(owned_ptr&& v) noexcept : _target(v.release()) {}

         template<typename U> owned_ptr& operator=(const owned_ptr<U>&) = delete;
         constexpr owned_ptr& operator=(owned_ptr&& v) noexcept {
            this->clear();
            this->_target = v.release();
            return *this;
         }

         constexpr ~owned_ptr() {
            this->clear();
         }

         constexpr void clear() {
            if (this->_target) {
               delete this->_target;
               this->_target = nullptr;
            }
         }

         constexpr value_type& operator*() const noexcept(noexcept(*std::declval<pointer>())) { return *this->_target; }
         constexpr pointer operator->() const noexcept { return this->_target; }

         explicit constexpr operator bool() const noexcept { return this->_target != nullptr; }

         // Operator overloads for this are a bit weird due to the precise logic of how the language 
         // works.
         // 
         //  - You need an overload for implicit conversions to the value type or any base classes. 
         //    Okay, fine.
         //
         //  - Even with that overload, however, you need an overload to allow comparisons to the 
         //    std::nullptr_t type, because the constant `nullptr` has no specific type otherwise 
         //    and evidently doesn't implicitly convert to your own pointer types in these cases.
         // 
         //     - What the hell? Why not?
         //
         //  - But if you have *both* overloads, then all pointer comparisons become ambiguous, 
         //    unless your conversion excludes std::nullptr_t specifically.
         //
         //  - If you don't overload operator== for non-null pointers, then direct comparisons 
         //    to `value_type*` or base classes fail. At one point, however, I was getting issues 
         //    with ambiguous comparisons here, but these stopped after excluding conversions to 
         //    std::nullptr_t.

         template<typename Base> requires (std::is_base_of_v<Base, value_type> && !std::is_null_pointer_v<Base> && !std::is_same_v<Base, void>)
         constexpr operator Base*() const noexcept { return (Base*)this->_target; }
         //
         template<typename Other> constexpr bool operator==(const Other* o) {
            return o == get();
         }
         constexpr bool operator==(std::nullptr_t) const { // this overload is still needed
            return get() == nullptr;
         }

         constexpr pointer get() const {
            return this->_target;
         }
         constexpr pointer release() {
            auto* p = this->_target;
            this->_target = nullptr;
            return p;
         }

         // Don't allow pointer arithmetic:
         template<typename U> owned_ptr& operator+(U) const = delete;
         template<typename U> owned_ptr& operator-(U) const = delete;
   };

   // std::swap will invoke this via ADL when given two owned_ptrs of the same specialization directly
   template<typename value_type> constexpr void swap(owned_ptr<value_type>& a, owned_ptr<value_type>& b) {
      std::swap(a._target, b._target);
   }

   // Don't allow pointer arithmetic:
   template<typename T, typename U> T operator+(const T&, const owned_ptr<U>&) = delete;
   template<typename T, typename U> T operator-(const T&, const owned_ptr<U>&) = delete;
}
