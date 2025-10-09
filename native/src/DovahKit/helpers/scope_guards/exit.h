#pragma once
#include <type_traits>

namespace cobb::scope_guards {
   template<typename Functor>
      requires requires(std::add_lvalue_reference_t<std::remove_reference_t<Functor>> functor) {
         { functor() };
      }
   class exit {
      private:
         Functor _functor;
         bool    _released = false;

      public:
         template<typename FunctorArg>
            requires (
               !std::is_same_v<std::remove_cvref_t<FunctorArg>, exit> &&
               std::is_constructible_v<Functor, FunctorArg>
            )
         explicit constexpr exit(FunctorArg&& functor)
            noexcept(std::is_nothrow_constructible_v<Functor, FunctorArg> || std::is_nothrow_constructible_v<Functor, FunctorArg&>)
         {
            try {
               if constexpr (!std::is_lvalue_reference_t<FunctorArg> && std::is_nothrow_constructible_v<Functor, FunctorArg>) {
                  this->_functor = std::forward<Functor>(functor);
               } else {
                  this->_functor = functor;
               }
            } catch (...) {
               (functor)();
            }
         }

         constexpr exit(exit&& other)
            noexcept(std::is_nothrow_move_constructible_v<Functor> || std::is_nothrow_copy_constructible_v<Functor>)
            requires (std::is_nothrow_move_constructible_v<Functor> || std::is_copy_constructible_v<Functor>)
         {
            this->_released = other._released;
            if constexpr (std::is_nothrow_move_constructible_v<Functor>)
               this->_functor = std::forward(other._functor);
            else
               this->_functor = other._functor;
            other.release();
         }

         constexpr ~exit() {
            if (this->_released)
               return;
            this->_released = true;
            (this->_functor)();
            this->_functor.~Functor();
         }

         exit(const exit&) = delete;

         exit& operator=(const exit&) = delete;
         exit& operator=(exit&&) = delete;

         constexpr void release() noexcept {
            this->_released = true;
         }
   };

   template<typename Functor>
   exit(Functor a) -> exit<Functor>;
}