#pragma once
#include <functional>
#include "../_ui_base.h"

namespace dovahscript::tasks::s2m {
   class ui_write_lambda : public _ui_write_base {
      protected:
         const bool _blocking;
      public:
         ui_write_lambda(bool b = false) : _blocking(b) {}

         std::function<void()> handler;
         
         virtual bool is_blocking() const noexcept override { return this->_blocking; }
      protected:
         virtual void _exec_impl() override {
            if (this->handler)
               (this->handler)();
         }
   };
}