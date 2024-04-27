#pragma once
#include <cstdint>
#include "../base_file_load_error.h"
#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class parent_form_is_missing : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         struct {
            uint32_t  local_id = 0;
            form_type type = form_type::none;
         } form;
   };
}
#include "../_util.undef.h"