#pragma once
#include "../bsa_parse_error.h"

namespace dovah::exceptions::bsa_parse_errors {
   class incorrect_header_sentinel : public bsa_parse_error {
      public:
         using bsa_parse_error::bsa_parse_error;
   };
}