#pragma once
#include <functional>
#include "../_base.h"

namespace dovahscript::tasks::s2m {
   class lambda : public _base {
      protected:
         const bool _blocking;
      public:
         lambda(bool b = true) : _blocking(b) {}

         std::function<void()> handler;
         
         virtual bool is_blocking() const noexcept override { return this->_blocking; }
      protected:
         virtual void _exec_impl() override {
            if (this->handler)
               (this->handler)();
         }
   };
}