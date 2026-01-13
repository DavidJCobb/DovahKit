#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../bare_form_id_t.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_name_is_unrecognized final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         std::string setting_name;
         struct {
            bare_form_id_t                local = 0;
            std::optional<bare_form_id_t> global;
         } form_ids;
   };
}
#include "../_util.undef.h"