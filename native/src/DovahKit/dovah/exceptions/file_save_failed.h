#pragma once
#include <memory>
#include <stdexcept>
#include "../notices/base_form_save_error.h"

namespace dovah {
   class form_stub;
}

namespace dovah::exceptions {
   class file_save_failed : public std::runtime_error {
      public:
         enum class error_code {
            // There is no active file to save.
            no_active_file,

            // No output filename was specified.
            no_filename_specified,

            // A save or load operation is already in progress.
            save_or_load_already_in_progress,

            file_has_too_many_dependencies,

            out_of_memory,
            zlib_memory_error,
            zlib_buffer_error,
            zlib_unknown_error,

            // Cannot save an ESL file: some of the forms in the active file lie outside 
            // the range of valid form IDs for light plugins.
            forms_out_of_esl_form_id_range,

            unimplemented_form_type,

            // A problem occurred while saving an individual form.
            form_save_failed,

            // The file was successfully saved, but couldn't be reopened to enable further 
            // editing. It is no longer save to use this `file_load_order`.
            save_complete_but_reopen_failed,

            unsaved_form_cleanup_failed,

            post_save_none_stub_cleanup_failed,
         };

      public:
         file_save_failed(error_code ec) : std::runtime_error("File save failed"), code(ec) {}

         // See: ./_docs/be careful with unique_ptr members.md
         file_save_failed(const file_save_failed& src) : std::runtime_error("File save failed") {
            *this = src;
         }
         file_save_failed& operator=(const file_save_failed& src) {
            this->code = src.code;
            this->details.unimplemented_form = src.details.unimplemented_form;
            if (auto* src_error = src.details.form_save_error.get())
               this->details.form_save_error.reset((notices::base_form_save_error*)src_error->clone());
            return *this;
         }

         error_code code;

         struct {
            std::unique_ptr<notices::base_form_save_error> form_save_error = nullptr;

            form_stub* unimplemented_form = nullptr;
         } details;
   };
}