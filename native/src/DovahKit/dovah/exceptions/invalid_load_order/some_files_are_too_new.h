#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "../invalid_load_order.h"

namespace dovah::exceptions::invalid_load_order_exceptions {
   class some_files_are_too_new : public invalid_load_order {
      public:
         some_files_are_too_new(std::vector<std::string>&& f) :
            invalid_load_order("The desired active file is a dependency of another file in the load order."),
            files(f)
         {}

         std::vector<std::string> files;
   };
}