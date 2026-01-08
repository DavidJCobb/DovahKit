#pragma once
#include <cstdint>
#include <vector>
#include "../../../../form_reference_t.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovah::loaded_forms::structs::region::generable_content {
   class weather_collection {
      public:
         struct weather_entry { // RDWT
            form_reference_t weather; // -> WTHR
            uint32_t         chance;
            form_reference_t global;  // -> GLOB
         };

      public:
         std::vector<weather_entry> weathers;

      public:
         void clear(dovah::loaded_forms::Form& my_containing_form);
         void clone_from(dovah::loaded_forms::Form& my_containing_form, const weather_collection&);
         void sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub&);
   };
}