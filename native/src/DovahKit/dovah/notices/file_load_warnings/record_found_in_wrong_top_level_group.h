#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class record_found_in_wrong_top_level_group final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         uint32_t top_level_group_label = 0; // the form type signature that we expected to see
         struct {
            bare_form_id_t                local_id = 0;
            std::optional<bare_form_id_t> global_id;
            uint32_t signature = 0;
         } record;
   };
}
#include "../_util.undef.h"