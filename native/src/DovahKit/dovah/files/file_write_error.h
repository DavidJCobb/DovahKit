#pragma once
#include <filesystem>
#include <string>
#include <type_traits>
#include "../core.h"

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
         struct error_code {
            error_code() = delete;
            enum type {
               none = 0,
               //
               // (unknown_form_type)
               // DovahKit doesn't know how to load, or how to save, the form type in question.
               //
               unknown_form_type,
               //
               // (no_active_file)
               // The user did not specify an active file, nor did they leave room for an implicit 
               // one in the load order.
               //
               no_active_file,
               //
               // (cannot_save_right_now)
               // It is not safe to save right now because some other operation (e.g. a load or 
               // save) is currently in progress.
               //
               cannot_save_right_now,
               //
               // (no_filename_specified)
               // The active file has no filename (i.e. it's implicit/invisible) and no filename 
               // was provided for it to use.
               //
               no_filename_specified,
               //
               // (save_complete_but_reopen_failed)
               // DovahKit was able to save the active file, but was not able to reopen it to 
               // continue editing. This means that we can no longer load form content for any 
               // form stubs from that file.
               //
               save_complete_but_reopen_failed,
            };
         };
         using error_code_t = std::underlying_type_t<error_code::type>;
         //
         error_code_t code = error_code::none;
         uint32_t     formID      = 0;
         uint32_t     file_offset = 0;
         form_type_t  form_type   = form_type::none;
         //
         inline bool defined() const noexcept { return this->code != error_code::none; }
         inline bool has_file_offset() const noexcept {
            switch (this->code) {
               case error_code::none:
               case error_code::no_active_file:
               case error_code::cannot_save_right_now:
               case error_code::no_filename_specified:
               case error_code::save_complete_but_reopen_failed:
                  return false;
            }
            return true;
         }
         inline bool requires_full_reload() const noexcept {
            switch (this->code) {
               case error_code::save_complete_but_reopen_failed:
                  return true;
            }
            return false;
         }
   };
}