#include "results.h"
#include "../../notices/base_file_load_warning.h"

namespace dovah::tes_file_reading {
   read_results::read_results() {
      this->error.context = detailed_notice::notice_context::file_load;
   }
   detailed_notice& read_results::add_warning() noexcept {
      auto& notice = this->warnings.emplace_back();
      notice.type    = detailed_notice::notice_type::warning;
      notice.context = detailed_notice::notice_context::file_load;
      return notice;
   }
   void read_results::add_warning(detailed_notice& n) noexcept {
      this->warnings.push_back(n);
   }
   void read_results::add_warning(const notices::base_file_load_warning& src) {
      this->warnings_ex.emplace_back().reset((notices::base_file_load_warning*)src.clone());
   }
}