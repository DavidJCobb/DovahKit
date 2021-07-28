#pragma once
#include <functional>
#include "../_ui_base.h"

namespace dovahscript::tasks::s2m {
   class ui_read_lambda : public _ui_read_base {
      public:
         std::function<void()> handler;
         
      protected:
         virtual void _exec_impl() override {
            if (this->handler)
               (this->handler)();
         }
   };
}