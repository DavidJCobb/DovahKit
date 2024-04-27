#pragma once
#include <stdexcept>
#include <string>

namespace dovah::exceptions::invalid_load_order_exceptions {
   class desired_active_file_is_a_dependency : public std::runtime_error {
      public:
         desired_active_file_is_a_dependency() : std::runtime_error("The desired active file is a dependency of another file in the load order.") {}

         std::string active_file;
         std::string dependent_file;
   };
}