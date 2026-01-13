#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../bare_form_id_t.h"
#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class singleton_form_is_redundantly_defined final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         bare_form_id_t previous_form_id = 0;
         struct {
            bare_form_id_t local_id  = 0;
            bare_form_id_t global_id = 0;
            enum form_type form_type = form_type::none;
         } record;
   };
}
#include "../_util.undef.h"