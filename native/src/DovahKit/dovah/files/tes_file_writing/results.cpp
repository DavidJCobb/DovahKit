#include "results.h"

namespace dovah::tes_file_writing {
   write_results::write_results() {
      this->error.context = detailed_notice::notice_context::file_save;
   }
}