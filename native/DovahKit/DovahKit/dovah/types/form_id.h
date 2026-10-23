#pragma once
#include "lu/utils/nominal_integer_type.h"
#include "./bare_form_id.h"
namespace dovah::tes {
   class subrecord_view;
}

namespace dovah {
   class form_id : public lu::utils::nominal_integer_type<form_id, bare_form_id> {
      private:
         using _my_base = lu::utils::nominal_integer_type<form_id, bare_form_id>;
      public:
         using integral_type = _my_base::integral_type;
         using _my_base::nominal_integer_type;

         void unchecked_stream(tes::subrecord_view&);
   };
}
