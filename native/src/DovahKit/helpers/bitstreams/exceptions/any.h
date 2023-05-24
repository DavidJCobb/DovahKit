#pragma once
#include <stdexcept>
#include <vector>
#include "../parse_step_info.h"

namespace cobb::bitstreams::exceptions {
   class any : public std::exception {
      public:
         std::vector<parse_step_info> where_were_we;
   };
}