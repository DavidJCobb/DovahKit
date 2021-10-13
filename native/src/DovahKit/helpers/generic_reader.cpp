#include "generic_reader.h"

namespace cobb {
   void generic_reader::set_data(const void* d, size_t s) {
      this->_data     = d;
      this->_size     = d ? s : 0;
      this->_position = 0;
   }
}