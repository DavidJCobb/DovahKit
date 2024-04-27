#pragma once
#include <cstdint>
#include "../base_file_load_error.h"
#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class form_override_has_type_mismatch : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         uint32_t form_id = 0;
         struct {
            uint32_t    local_id = 0;
            form_type   type     = form_type::none;
            std::string source_file;
         } overriding_form;
         struct {
            form_type   type = form_type::none;
            std::string source_file;
         } overridden_form;
   };
}
#include "../_util.undef.h"