#pragma once
#include "core.h"
#include "notice_code_t.h"
#include <string>
#include <vector>

namespace dovah {
   class form_reference_t;
   class form_stub;

   struct detailed_notice {
      enum class notice_context {
         unspecified         = 0,
         on_demand_form_load = 1,
         file_save           = 2,
         form_save           = 3,
         file_load           = 4,
      };
      enum class notice_type {
         unspecified = 0,
         warning     = 1,
         error       = 2,
      };

      struct flag {
         flag() = delete;
         enum type {
            has_file_offset           = 0x00000001,
            has_cause_form            = 0x00000002,
            has_cause_file            = 0x00000004,
            has_cause_subrecord       = 0x00000008,
            has_cause_form_type       = 0x00000010,
            has_cause_subrecord_index = 0x00000020,
            is_winning_record         = 0x00000040, // This warning was generated while loading form data from the last-loaded record for the cause form.
            is_coalesced_record_data  = 0x00000080, // This warning applies to form data that is coalesced across multiple files/overrides.
            has_cause_form_index      = 0x00000100, // This warning applies to the Nth form provided by one or more cause subrecords.
            has_cause_editor_id       = 0x00000200,
            has_cause_signature       = 0x00000400,
            has_cause_size            = 0x00000800,
            has_errno                 = 0x00001000,
            has_winapi_error_code     = 0x00002000,
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      struct relevant_form {
         bare_form_id_t localID = 0; // file-local form ID, when available; zero otherwise
         bare_form_id_t fixedID = 0; // load-order-relative form ID, when available; zero otherwise
         form_type      type    = form_type::none;
         //
         inline bool operator==(const relevant_form& other) const noexcept {
            return (this->localID == other.localID) && (this->fixedID == other.fixedID) && (this->type == other.type);
         }
         inline bool operator!=(const relevant_form& other) const noexcept { return !(*this == other); }
      };

      notice_type    type    = notice_type::unspecified;
      notice_context context = notice_context::unspecified;
      notice_code_t  code    = default_notice_code;
      flags_t        flags   = 0;
      uint32_t       offset  = 0;
      uint32_t       cause_subrecord       = 0;
      int            cause_subrecord_index = 0;
      int            cause_form_index      = 0;
      relevant_form  cause_form; // the form in which the error occurred
      std::string    cause_file;
      std::string    cause_editor_id;
      form_type      cause_form_type = form_type::none; // this IS NOT the same thing as the "cause form's type." if for example some form X referred to a form Y and Y had the wrong type, this would be the type X was expecting.
      uint32_t       cause_signature = 0;
      uint32_t       cause_size      = 0;
      errno_t        errno_value     = 0;
      uint32_t       winapi_error    = 0;
      std::vector<relevant_form> relevant_forms;
      std::vector<std::string>   relevant_files;
      std::vector<std::string>   relevant_strings;
      std::array<uint32_t, 4>    extra_integers = {};

      void add_relevant_form(const form_stub&);

      static detailed_notice warn_about_unrecognized_subrecord(uint32_t subrecord, const form_stub& referrer);
      static detailed_notice warn_if_wrong_type(uint32_t subrecord_signature, form_type desired, const form_stub& referrer, const form_reference_t& reference);
      static detailed_notice warn_if_wrong_type(uint32_t subrecord_signature, std::initializer_list<form_type> desired, const form_stub& referrer, const form_reference_t& reference);
      static detailed_notice warn_if_not_object_reference(uint32_t subrecord_signature, const form_stub& referrer, const form_reference_t& reference);

      // Chainable setters:
      inline detailed_notice& modify_flag(flags_t f, bool s) noexcept {
         if (s)
            this->flags |= f;
         else
            this->flags &= ~f;
         return *this;
      }
      inline detailed_notice& set_flag(flags_t f) noexcept {
         this->flags |= f;
         return *this;
      }
      detailed_notice& set_cause_file(const std::string& filename);
      detailed_notice& set_cause_form(const form_stub&);
      detailed_notice& set_cause_form_type(form_type);
      detailed_notice& set_cause_subrecord(uint32_t signature);
      detailed_notice& set_cause_editor_id(const std::string&);
      detailed_notice& set_cause_form_index(int) noexcept;
      detailed_notice& set_cause_signature(uint32_t) noexcept;
      detailed_notice& set_cause_size(uint32_t) noexcept;
      detailed_notice& set_errno(errno_t) noexcept;
      detailed_notice& set_file_offset(uint32_t) noexcept;
      detailed_notice& set_subrecord_index(int) noexcept;
      detailed_notice& set_winapi_error_code(uint32_t) noexcept;

      detailed_notice& add_relevant_file(const std::string& filename);

      inline bool is_defined() const noexcept { return this->code != default_notice_code; } // making this (operator bool) would be cool except that that breaks equality comparisons because this language sucks sometimes
      inline bool is_winning_record() const noexcept { return this->flags & (flag::is_winning_record | flag::is_coalesced_record_data); }

      bool operator==(const detailed_notice&) const noexcept;
      inline bool operator!=(const detailed_notice& other) const noexcept { return !(*this == other); }
   };
}