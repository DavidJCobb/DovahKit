#pragma once
#include "./base_group_error.h"

namespace dovah::exceptions::file_parse_errors {
   class interior_cell_block_group_badly_nested : public base_group_error {
      public:
         using base_group_error::base_group_error;
   };
}