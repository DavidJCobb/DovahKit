#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include "../invalid_load_order.h"

namespace dovah::exceptions::invalid_load_order_exceptions {
   class load_order_would_have_too_many_files : public invalid_load_order {
      public:
         load_order_would_have_too_many_files() : invalid_load_order("Accounting for files' dependencies, this load order would contain too many files.") {}

         struct {
            std::optional<size_t> active_file_dependencies;
            size_t light = 0;
            size_t heavy = 0;
            size_t total = 0;
         } file_counts;
         std::string overflowed_at_file;
   };
}