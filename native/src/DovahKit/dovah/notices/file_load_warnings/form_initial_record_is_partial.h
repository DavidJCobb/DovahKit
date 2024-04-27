#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class form_initial_record_is_partial final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         // Skyrim will skip loading a record if it's partial, injected, and not an override. So does DovahKit.
         bool record_is_injected = false;

         struct {
            bare_form_id_t local_id  = 0;
            bare_form_id_t global_id = 0;
            enum form_type form_type = form_type::none;
         } record;
   };
}
#include "../_util.undef.h"