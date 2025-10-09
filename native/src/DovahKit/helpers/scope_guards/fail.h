#pragma once
#include <exception>
#include <type_traits>
#if __cpp_lib_uncaught_exceptions < 201411L
   #error This class requires support for std::uncaught_exceptions(), i.e. the one with an "S" that returns an int.
#endif

namespace cobb::scope_guards {
   template<typename Functor>
      requires requires(std::add_lvalue_reference_t<std::remove_reference_t<Functor>> functor) {
         { functor() };
      }
   class fail {
      private:
         Functor _functor;
         bool    _released = false;
         const int _uncaught_at_construction;

      public:
         template<typename FunctorArg>
            requires (
               !std::is_same_v<std::remove_cvref_t<FunctorArg>, fail> &&
               std::is_constructible_v<Functor, FunctorArg>
            )
         explicit constexpr fail(FunctorArg&& functor)
            noexcept(std::is_nothrow_constructible_v<Functor, FunctorArg> || std::is_nothrow_constructible_v<Functor, FunctorArg&>)
         :
            _uncaught_at_construction(std::uncaught_exceptions())
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

         constexpr fail(fail&& other)
            noexcept(std::is_nothrow_move_constructible_v<Functor> || std::is_nothrow_copy_constructible_v<Functor>)
            requires (std::is_nothrow_move_constructible_v<Functor> || std::is_copy_constructible_v<Functor>)
         :
            _uncaught_at_construction(std::uncaught_exceptions())
         {
            this->_released = other._released;
            if constexpr (std::is_nothrow_move_constructible_v<Functor>)
               this->_functor = std::forward(other._functor);
            else
               this->_functor = other._functor;
            other.release();
         }

         constexpr ~fail() {
            if (this->_released)
               return;
            this->_released = true;
            if (this->_uncaught_at_construction > std::uncaught_exceptions()) {
               (this->_functor)();
            }
            this->_functor.~Functor();
         }

         fail(const fail&) = delete;

         fail& operator=(const fail&) = delete;
         fail& operator=(fail&&) = delete;

         constexpr void release() noexcept {
            this->_released = true;
         }
   };

   template<typename Functor>
   fail(Functor a) -> fail<Functor>;
}