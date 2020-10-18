#pragma once
#include <filesystem>
#include <string>
#include <type_traits>
#include "../core.h"

//
// Refer to <_docs/error code documentation/file_write_error.txt> for a list of specific notice 
// codes that can appear here.
//

namespace dovah {
   class file_write_warning {
      public:
         struct warning_code {
            warning_code() = delete;
            enum type {
               none = 0,
               //
               // (save_complete_but_to_temporary_file)
               // DovahKit was able to save a temporary file, but was unable to swap the old 
               // active file out.
               //
               save_complete_but_to_temporary_file,
            };
         };
         using warning_code_t = std::underlying_type_t<warning_code::type>;
         //
         warning_code_t code = warning_code::none;
         std::filesystem::path filename;
         //
         inline bool defined() const noexcept { return this->code != warning_code::none; }
   };
   class file_write_error {
      public:
         notice_code_t code        = 0;
         uint32_t      formID      = 0;
         uint32_t      file_offset = 0;
         form_type_t   form_type   = form_type::none;
         //
         inline bool defined() const noexcept { return this->code; }
         bool has_file_offset() const noexcept;
         bool requires_full_reload() const noexcept;
   };
}