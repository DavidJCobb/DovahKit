#pragma once
#include <stdexcept>
#include <string>
#include <vector>

namespace dovah::exceptions::invalid_load_order_exceptions {
   class cyclical_dependency_between_files : public std::runtime_error {
      public:
         cyclical_dependency_between_files() : std::runtime_error("This load order contains a cyclical dependency.") {}

         std::vector<std::string> seen;
   };
}