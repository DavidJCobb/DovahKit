#pragma once
#include <expected>
#include "dovah/types/bare_form_id.h"
namespace dovah {
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::utils::form_ids {
   enum class record_id_to_form_id_error {
      missing_master,
      out_of_bounds,
   };

   extern std::expected<bare_form_id, record_id_to_form_id_error> record_id_to_form_id(const tes::file& id_read_from, bare_form_id record_id);
}