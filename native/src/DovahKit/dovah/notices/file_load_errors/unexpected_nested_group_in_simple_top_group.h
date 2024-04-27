#pragma once
#include "../base_file_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class unexpected_nested_group_in_simple_top_group : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
   };
}
#include "../_util.undef.h"