#pragma once
#include <stdexcept>
#include <string>
#include "../invalid_load_order.h"

namespace dovah::exceptions::invalid_load_order_exceptions {
   class desired_active_file_is_a_dependency : public invalid_load_order {
      public:
         desired_active_file_is_a_dependency() : invalid_load_order("The desired active file is a dependency of another file in the load order.") {}

         std::string active_file;
         std::string dependent_file;
   };
}