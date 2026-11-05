#pragma once
namespace dovah {
   namespace notices {
      class base_warning;
   }
   class form_stub;
}

namespace dovah::callback_sets {
   struct post_load {
      public:
         using on_warning_type      = void(*)(const notices::base_warning&);
         using on_form_created_type = void(*)(form_stub&);
         using on_form_lost_type    = void(*)(form_stub&);
      public:
         on_form_created_type on_form_created = nullptr;
         on_form_lost_type    on_form_lost    = nullptr; // a form stub is being forcibly deleted by backend processes (e.g. none-stub being clobbered; SSE-only form when re-saving to LE)
         on_warning_type      on_warning      = nullptr;
   };
}