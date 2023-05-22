#pragma once
#include <stdexcept>

namespace cobb::bitstreams::exceptions {
   class missing_data_header : public std::exception {
   };
}
