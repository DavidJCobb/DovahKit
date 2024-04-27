#pragma once
#include <memory>
#include <stdexcept>
#include "../notices/base_file_load_error.h"

namespace dovah::exceptions {
   class file_load_failed : public std::runtime_error {
      public:
         enum class error_code {
            // A save or load operation is already in progress.
            save_or_load_already_in_progress,

            no_filename_specified,

            see_details_object,
         };

      public:
         file_load_failed() : std::runtime_error("File load failed") {}
         file_load_failed(error_code ec) : std::runtime_error("File load failed"), code(ec) {}

         // See: ./_docs/be careful with unique_ptr members.md
         file_load_failed(const file_load_failed& src) : std::runtime_error("File load failed") {
            *this = src;
         }
         file_load_failed& operator=(const file_load_failed& src) {
            this->code = src.code;
            if (auto* src_error = src.details.file_load_error.get())
               this->details.file_load_error.reset((notices::base_file_load_error*)src_error->clone());
            return *this;
         }

         error_code code = error_code::see_details_object;

         struct {
            std::unique_ptr<notices::base_file_load_error> file_load_error = nullptr;
         } details;
   };
}