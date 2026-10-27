#pragma once
namespace dovah {
   namespace notices {
      class base_warning;
   }
}

namespace dovah::callback_sets {
   struct post_load {
      public:
         using on_warning_type = void(*)(const notices::base_warning&);
      public:
         on_warning_type on_warning = nullptr;
   };
}