#include "./record_id_to_form_id.h"
#include <cassert>
#include "dovah/active_load_order.h"
#include "dovah/facts/form_ids/max_hardcoded_form_id.h"
#include "dovah/facts/game/hardcoded_form_ids_ignore_record_id_prefix.h"
#include "dovah/tes/file.h"
#include "dovah/types/file_prefix.h"

namespace dovah::utils::form_ids {
   extern std::expected<bare_form_id, record_id_to_form_id_error> record_id_to_form_id(const tes::file& source_file, bare_form_id record_id) {
      if (record_id == 0) {
         return 0;
      }

      auto* alo_ptr = source_file.get_active_load_order();
      assert(!!alo_ptr);
      auto& alo     = *alo_ptr;

      if (facts::hardcoded_form_ids_ignore_record_id_prefix(alo.get_current_game(), source_file.header().file_version)) {
         auto id_to_test = record_id & 0x00FFFFFF;
         if (id_to_test <= facts::form_ids::max_hardcoded_form_id) {
            return id_to_test;
         }
      } else {
         if (record_id <= facts::form_ids::max_hardcoded_form_id) {
            return record_id;
         }
      }
      uint8_t local  = source_file.header().masters.size();
      uint8_t prefix = record_id >> 0x18;
      if (prefix == local) {
         return alo.file_prefix_for(source_file).coerce_form_id(record_id);
      }
      if (prefix > local) {
         return std::unexpected(record_id_to_form_id_error::out_of_bounds);
      }
      auto& name        = source_file.header().masters[prefix].master;
      auto  file_prefix = alo.file_prefix_for(name);
      if (file_prefix.is_undefined()) [[unlikely]] {
         return std::unexpected(record_id_to_form_id_error::missing_master);
      }
      return file_prefix.coerce_form_id(record_id);
   }
}