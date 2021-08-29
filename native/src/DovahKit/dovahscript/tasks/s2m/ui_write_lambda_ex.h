#pragma once
#include <type_traits>
#include "../_ui_base.h"

namespace dovahscript::tasks::s2m {
   //
   // A templated task which allows you to supply a lambda for task behavior. The use of 
   // templating in favor of std::function allows us to avoid redundant copies of the 
   // lambda itself and of anything it has captured.
   //
   template<typename lambda_type> requires std::is_invocable_v<lambda_type>
   class ui_write_lambda_ex : public _ui_write_base {
      public:
         ui_write_lambda_ex(bool blocking, lambda_type&& l) : _blocking(blocking), _lambda(std::move(l)) {}

      protected:
         const bool  _blocking;
         lambda_type _lambda;

         virtual void _exec_impl() override {
            (this->_lambda)();
         }
      public:
         virtual bool is_blocking() const noexcept override { return this->_blocking; }
   };
}