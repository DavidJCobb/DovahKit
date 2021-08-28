#pragma once
#pragma once
#include "../_base.h"
#include "../../../dovah/core.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript::tasks::s2m {
   class renumber_form : public _base {
      public:
         dovah::form_stub*     stub = nullptr;
         dovah::bare_form_id_t desiredID = 0;
         bool        error      = false;
         const char* error_text = nullptr;
         //
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}