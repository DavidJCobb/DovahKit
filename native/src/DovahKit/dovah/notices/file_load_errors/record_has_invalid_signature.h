#pragma once
#include "./base_record_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class record_has_invalid_signature : public base_record_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr record_has_invalid_signature() {}
   };
}
#include "../_util.undef.h"