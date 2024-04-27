#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_error.h"

namespace dovah::notices::file_load_errors {
   class base_record_load_error : public base_file_load_error {
      public:
         constexpr base_record_load_error() {}

         struct {
            uint32_t signature     = 0;
            uint32_t local_form_id = 0;
            std::optional<uint32_t> resolved_form_id;
         } record;
   };
}