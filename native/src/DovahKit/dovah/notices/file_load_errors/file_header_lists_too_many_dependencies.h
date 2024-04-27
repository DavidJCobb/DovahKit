#pragma once
#include "../base_file_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class file_header_lists_too_many_dependencies : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
   };
}
#include "../_util.undef.h"