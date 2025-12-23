#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "../invalid_load_order.h"

namespace dovah::exceptions::invalid_load_order_exceptions {
   class cyclical_dependency_between_files : public invalid_load_order {
      public:
         cyclical_dependency_between_files() : invalid_load_order("This load order contains a cyclical dependency.") {}

         std::vector<std::string> seen;
   };
}