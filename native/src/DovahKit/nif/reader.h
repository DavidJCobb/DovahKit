#pragma once
#include <string>
#include "file.h"
#include "helpers/generic_reader.h"

namespace nifDK {
   class file_reader : public cobb::generic_reader {
      public:
         using cobb::generic_reader::generic_reader;
         using cobb::generic_reader::generic_reader::read;
         using cobb::generic_reader::generic_reader::unchecked_read;

         bool read(file_version&);
         void unchecked_read(file_version&);

         bool read_line_string(std::string& out) {
            uint8_t byte;
            bool    line = false;
            while (this->read(byte)) {
               if (byte == '\n' || byte == '\00') {
                  break;
               }
               if (byte == '\r') {
                  line = true;
                  continue;
               }
               if (line)
                  return false; // '\r' not followed by '\n'
               out.push_back(byte);
            }
            return true;
         }
   };
}