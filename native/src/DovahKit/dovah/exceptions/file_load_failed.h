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

         error_code code = error_code::see_details_object;

         struct {
            std::unique_ptr<notices::base_file_load_error> file_load_error = nullptr;
         } details;
   };
}