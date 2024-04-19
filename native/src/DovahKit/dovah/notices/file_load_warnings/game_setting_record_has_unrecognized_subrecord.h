#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_record_has_unrecognized_subrecord final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         std::string setting_name;
         struct {
            bare_form_id_t                local = 0;
            std::optional<bare_form_id_t> global; // unlikely to be available, since we emit this warning before resolving form IDs
         } form_ids;
         uint32_t subrecord_signature = 0;
   };
}
#include "../_util.undef.h"