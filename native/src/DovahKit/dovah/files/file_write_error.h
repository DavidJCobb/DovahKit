#pragma once
#include <string>
#include <type_traits>
#include "../core.h"

namespace dovah {
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
   };
}