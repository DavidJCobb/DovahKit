#pragma once
#include "../_base.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   //
   // APIs must emit this task before and after modifying a form. They should not emit it directly, 
   // but rather should rely on wrapper::before_edit and wrapper::after_edit, which will handle this 
   // task and any Lua-side operations needed.
   //
   class internal_signal_form_edit final : public _base {
      public:
         internal_signal_form_edit();
         internal_signal_form_edit(dovah::form_stub& stub, bool before);

      public:
         dovah::form_stub* stub = nullptr;
         bool before = false;
         
         virtual bool is_blocking() const noexcept override final { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}