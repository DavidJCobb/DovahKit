#pragma once
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class keyword_list;
      }
      class Form;
   }
   class form_stub;
}

namespace dovah::utils::form_component_accessors {
   extern const loaded_forms::components::keyword_list* keyword_list(const dovah::loaded_forms::Form&);
   extern loaded_forms::components::keyword_list* keyword_list(dovah::loaded_forms::Form&);
   extern loaded_forms::components::keyword_list* keyword_list(dovah::form_stub&);
}