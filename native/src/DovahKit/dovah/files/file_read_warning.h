#pragma once
#include "../core.h"
#include "../notice_code_t.h"
#include <string>
#include <vector>

namespace dovah {
   class form_reference_t;
   class form_stub;

   struct file_read_warning {
      struct flag {
         flag() = delete;
         enum type {
            has_file_offset     = 0x00000001,
            has_cause_form      = 0x00000002,
            has_cause_file      = 0x00000004,
            has_cause_subrecord = 0x00000008,
            has_cause_form_type = 0x00000010,
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
      uint32_t      cause_subrecord = 0;
      relevant_form cause_form; // the form in which the error occurred
      std::string   cause_file;
      form_type_t   cause_form_type = form_type::none;
      std::vector<relevant_form> relevant_forms;
      std::vector<std::string>   relevant_files;
      std::array<uint32_t, 4>    extra_integers;

      void set_cause_form(const form_stub&);
      void set_cause_form_type(form_type_t);
      void set_cause_subrecord(uint32_t signature);

      inline void set_flag(flags_t f) noexcept { this->flags |= f; }

      void add_relevant_form(const form_stub&);

      static file_read_warning warn_about_unrecognized_subrecord(uint32_t subrecord, const form_stub& referrer);
      static file_read_warning warn_if_wrong_type(uint32_t subrecord_signature, form_type_t desired, const form_stub& referrer, const form_reference_t& reference);

      inline operator bool() const noexcept { return this->code != default_notice_code; }
   };
}