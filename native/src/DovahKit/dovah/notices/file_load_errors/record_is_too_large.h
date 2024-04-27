#pragma once
#include "./base_record_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class record_is_too_large : public base_record_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
   };
}
#include "../_util.undef.h"