#pragma once
#include "../core.h"
#include "../notice_code_t.h"
#include <string>
#include <vector>

namespace dovah {
   class form_reference_t;
   class form_stub;

   struct file_save_warning {
      struct flag {
         flag() = delete;
         enum type {
            has_cause_form            = 0x00000001,
            has_cause_subrecord       = 0x00000002,
            has_cause_form_type       = 0x00000004,
            has_cause_subrecord_index = 0x00000008,
            has_cause_form_index      = 0x00000010, // This warning applies to the Nth form provided by one or more cause subrecords.
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;

      enum class context_t {
         unspecified,
         on_demand_form_save,
      };

      struct relevant_form {
         bare_form_id_t fixedID = 0;
         form_type_t    type    = form_type::none;
         //
         inline bool operator==(const relevant_form& other) const noexcept {
            return (this->fixedID == other.fixedID) && (this->type == other.type);
         }
         inline bool operator!=(const relevant_form& other) const noexcept { return !(*this == other); }
      };

      notice_code_t code    = default_notice_code;
      flags_t       flags   = 0;
      uint32_t      offset  = 0;
      context_t     context = context_t::unspecified;
      uint32_t      cause_subrecord       = 0;
      int           cause_subrecord_index = 0;
      int           cause_form_index      = 0;
      relevant_form cause_form; // the form in which the error occurred
      form_type_t   cause_form_type = form_type::none; // this IS NOT the same thing as the "cause form's type." if for example some form X referred to a form Y and Y had the wrong type, this would be the type X was expecting.
      std::vector<relevant_form> relevant_forms;
      std::vector<std::string>   relevant_files;
      std::array<uint32_t, 4>    extra_integers = {};

      void set_cause_form(const form_stub&);
      void set_cause_form_type(form_type_t);
      void set_cause_subrecord(uint32_t signature);

      void add_relevant_form(const form_stub&);

      // Chainable setters:
      inline file_save_warning& modify_flag(flags_t f, bool s) noexcept {
         if (s)
            this->flags |= f;
         else
            this->flags &= ~f;
         return *this;
      }
      inline file_save_warning& set_flag(flags_t f) noexcept {
         this->flags |= f;
         return *this;
      }
      file_save_warning& set_cause_form_index(int) noexcept;
      file_save_warning& set_subrecord_index(int) noexcept;

      inline bool is_defined() const noexcept { return this->code != default_notice_code; } // making this (operator bool) would be cool except that that breaks equality comparisons because this language sucks sometimes

      bool operator==(const file_save_warning&) const noexcept;
      inline bool operator!=(const file_save_warning& other) const noexcept { return !(*this == other); }
   };
}