#pragma once
#include <stdexcept>

namespace cobb::bitstreams::exceptions {
   class read_past_end : public std::exception {
   };
}
