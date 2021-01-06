#pragma once
#include "../base.h"

namespace dovah {
   class form_stub;
}

namespace editor_script::tasks::s2m {
   class delete_form : public cross_thread_task {
      public:
         dovah::form_stub* stub = nullptr;
         bool error = false;
         const char* error_text = nullptr;
         //
         virtual bool is_blocking() const noexcept override { return true; }
      protected:
         virtual void _exec_impl() override;
   };
}