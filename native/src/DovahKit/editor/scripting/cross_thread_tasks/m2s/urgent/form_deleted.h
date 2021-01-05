#pragma once
#include "../../base.h"

namespace dovah {
   class form_stub;
}

namespace editor_script::tasks::m2s {
   class form_deleted : public cross_thread_task {
      //
      // A callback which runs when a form is deleted. This isn't useful for the form:delete() 
      // API, as that has to zombify wrappers *before* carrying out the deletion operation in 
      // order to clear all pointers to the loaded-form data before the deletion occurs.
      //
      // Really, this is just here as a "just in case" thing.
      //
      public:
         dovah::form_stub* stub = nullptr;
         //
      protected:
         virtual void _exec_impl() override;
   };
}