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
      public:
         on_form_created_type on_form_created = nullptr;
         on_warning_type      on_warning      = nullptr;
   };
}