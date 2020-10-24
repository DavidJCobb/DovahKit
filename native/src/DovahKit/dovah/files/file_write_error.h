#pragma once
#include <filesystem>
#include <string>
#include <type_traits>
#include "../core.h"
#include "../notice_code_t.h"

//
// Refer to <_docs/error code documentation/file_write_error.txt> for a list of specific notice 
// codes that can appear here.
//

namespace dovah {
   class file_write_warning {
      public:
         notice_code_t code = default_notice_code;
         std::filesystem::path filename;
         //
         inline bool defined() const noexcept { return this->code; }
   };
   class file_write_error {
      public:
         notice_code_t code        = default_notice_code;
         uint32_t      formID      = 0;
         uint32_t      file_offset = 0;
         form_type_t   form_type   = form_type::none;
         //
         inline bool defined() const noexcept { return this->code; }
         bool has_file_offset() const noexcept;
         bool requires_full_reload() const noexcept;
   };
}