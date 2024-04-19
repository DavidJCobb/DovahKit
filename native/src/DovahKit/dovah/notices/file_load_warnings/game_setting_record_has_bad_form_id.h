#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_record_has_bad_form_id final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         struct {
            bare_form_id_t                local = 0;
            std::optional<bare_form_id_t> global; // invalid local ID if empty; invalid in itself if zero
         } form_ids;
         std::string setting_name; // invalid if empty
   };
}
#include "../_util.undef.h"