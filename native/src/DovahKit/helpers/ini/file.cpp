#pragma once
#include "./file.h"

namespace cobb::ini {
   void file::load(std::istream& src) {
      return this->_load(_input_stream<std::istream>{src});
   }
   void file::save(std::ostream& dst) {
      return this->_save(_output_stream<std::ostream>{dst});
   }
   void file::save(std::ostream& dst, std::istream& src) {
      return this->_save(_output_stream<std::ostream>{dst}, _input_stream<std::istream>{src});
   }
   void file::save(std::string& dst, std::istream& src) {
      return this->_save(_output_stream<std::string>{dst}, _input_stream<std::istream>{src});
   }
}