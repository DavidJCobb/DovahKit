#pragma once
namespace dovah {
   class form_stub;
}

namespace dovah {
   class base_managed_form_data {
      protected:
         constexpr base_managed_form_data(form_stub& stub) : stub(stub) {}
      public:
         virtual ~base_managed_form_data() {}

      public:
         form_stub& stub;
   };
}