#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_error.h"
#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class filesystem_error : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         std::optional<errno_t>  errno_value;
         std::optional<uint32_t> winapi_error;
   };
}
#include "../_util.undef.h"