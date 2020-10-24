#pragma once
#include "../core.h"
#include "../notice_code_t.h"
#include <string>
#include <vector>

namespace dovah {
   struct file_read_warning {
      struct flag {
         flag() = delete;
         enum type {
            has_file_offset = 0x00000001,
            has_cause_form  = 0x00000002,
            has_cause_file  = 0x00000004,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      struct relevant_form {
         bare_form_id_t localID = 0;
         bare_form_id_t fixedID = 0;
         form_type_t    type    = form_type::none;
      };

      notice_code_t code   = default_notice_code;
      flags_t       flags  = 0;
      uint32_t      offset = 0;
      relevant_form cause_form; // the form in which the error occurred
      std::string   cause_file;
      std::vector<relevant_form> relevant_forms;
      std::vector<std::string>   relevant_files;

      inline void set_flag(flags_t f) noexcept { this->flags |= f; }
   };
}