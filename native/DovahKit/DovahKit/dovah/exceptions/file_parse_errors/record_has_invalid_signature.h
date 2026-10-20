#pragma once
#include "./base_record_error.h"

namespace dovah::exceptions::file_parse_errors {
   class record_has_invalid_signature : public base_record_error {
      public:
         using base_record_error::base_record_error;
   };
}