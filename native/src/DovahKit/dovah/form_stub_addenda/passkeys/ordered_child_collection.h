#pragma once
namespace dovah {
   class form_stub;
   struct form_stub_addenda;
}

namespace dovah::form_stub_addendum_types::passkeys {
   class reorder_children_after_load {
      friend dovah::form_stub;
      friend dovah::form_stub_addenda;
      private:
         constexpr reorder_children_after_load() {}
   };
}