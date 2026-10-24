#pragma once
#include "../file_load_error.h"
#include "dovah/types/form_id.h"

namespace dovah::exceptions::file_load_errors {
   class missing_master_detected_at_record_load : public file_load_error {
      public:
         missing_master_detected_at_record_load(
            const std::filesystem::path& file,
            const std::string& master_filename,
            bare_form_id record_id
         ) :
            file_load_error(file),
            master_filename(master_filename),
            record_id(record_id)
         {}

      public:
         std::string  master_filename;
         bare_form_id record_id;
   };
}