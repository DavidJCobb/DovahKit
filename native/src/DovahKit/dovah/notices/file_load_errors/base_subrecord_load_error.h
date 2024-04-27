#pragma once
#include "./base_record_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class base_subrecord_load_error : public base_record_load_error {
      public:
         constexpr base_subrecord_load_error() {}

         struct {
            uint32_t signature = 0;
            uint32_t size      = 0;

            uint32_t offset_into_record = 0;
         } subrecord;
   };
}
#include "../_util.undef.h"