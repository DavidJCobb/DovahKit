#pragma once
#include <vector>
#include "../../../../form_reference_t.h"
namespace dovah::loaded_forms {
   class Form;
}

namespace dovah::loaded_forms::structs::region::generable_content {
   class grass_collection {
      public:
         struct entry { // RDGS
            form_reference_t grass;        // -> GRAS
            form_reference_t land_texture; // -> LTEX
         };

      public:
         std::vector<entry> entries;

      public:
         void clear(dovah::loaded_forms::Form& my_containing_form);
         void clone_from(dovah::loaded_forms::Form& my_containing_form, const grass_collection&);
         void sever_references_to(dovah::loaded_forms::Form& my_containing_form, dovah::form_stub&);
   };
}
