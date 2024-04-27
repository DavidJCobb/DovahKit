#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_value_has_extra_content final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         std::string setting_name;
         size_t      data_size     = 0;
         size_t      expected_size = 0;
         struct {
            bare_form_id_t                local_id = 0;
            std::optional<bare_form_id_t> global_id;
         } record;
   };
}
#include "../_util.undef.h"