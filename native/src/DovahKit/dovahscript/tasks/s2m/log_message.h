#pragma once
#include <QString>
#include "../_base.h"

namespace dovahscript::tasks::s2m {
   class log_message : public _base {
      public:
         log_message() {}
         log_message(QString t) : text(t) {}

         QString text;
         
         virtual bool is_blocking() const noexcept override { return false; }
      protected:
         virtual void _exec_impl() override;
   };
}