#pragma once
#include <optional>
#include <stdexcept>
#include <string>

namespace dovah::exceptions::invalid_load_order_exceptions {
   class load_order_would_have_too_many_files : public std::runtime_error {
      public:
         load_order_would_have_too_many_files() : std::runtime_error("Accounting for files' dependencies, this load order would contain too many files.") {}

         struct {
            std::optional<size_t> active_file_dependencies;
            size_t light = 0;
            size_t heavy = 0;
            size_t total = 0;
         } file_counts;
         std::string overflowed_at_file;
   };
}