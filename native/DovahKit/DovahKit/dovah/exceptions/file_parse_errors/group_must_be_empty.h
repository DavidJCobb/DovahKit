#pragma once
#include "./base_group_error.h"

namespace dovah::exceptions::file_parse_errors {
   // Official game files include GRUPs for form types that have been wholly deleted 
   // from the executable, i.e. CLDC. We don't even try parsing the innards of these 
   // GRUPs; we simply require that they be empty.
   class group_must_be_empty : public base_group_error {
      public:
         using base_group_error::base_group_error;
   };
}