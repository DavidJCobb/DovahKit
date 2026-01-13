#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include "../base_file_load_warning.h"
#include "../../bare_form_id_t.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_has_multiple_records_in_a_file final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         std::string setting_name;

         bare_form_id_t last_seen_form_id = 0;
         struct {
            bare_form_id_t                local = 0;
            std::optional<bare_form_id_t> global;
         } current_form_ids;
   };
}
#include "../_util.undef.h"