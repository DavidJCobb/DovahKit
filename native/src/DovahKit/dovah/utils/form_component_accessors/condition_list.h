#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class condition_list;
      }
      class Form;
   }
   class form_stub;
}

namespace dovah::utils::form_component_accessors {
   extern const loaded_forms::components::condition_list* condition_list(const dovah::loaded_forms::Form&);
   extern loaded_forms::components::condition_list* condition_list(dovah::loaded_forms::Form&);
}