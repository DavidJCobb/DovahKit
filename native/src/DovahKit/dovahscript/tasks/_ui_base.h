#pragma once
#include "_base.h"

namespace dovahscript::tasks {
   class _ui_read_base : virtual public _base {
      public:
         virtual bool is_blocking() const noexcept final { return true; }
   };

   class _ui_write_base : virtual public _base {};
}