#pragma once
#include "./base_subrecord_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class extended_subrecord_marker_is_invalid : public base_subrecord_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr extended_subrecord_marker_is_invalid() {
            this->subrecord.signature = 'XXXX';
         }
   };
}
#include "../_util.undef.h"