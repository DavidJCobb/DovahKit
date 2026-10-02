#pragma once
#include <stdio.h>

namespace lu::files {
   class file_pointer_guard {
      protected:
         FILE* _file = nullptr;

      public:
         constexpr file_pointer_guard() {}
         file_pointer_guard(FILE* f) : _file(f) {}
         ~file_pointer_guard() {
            if (this->_file) {
               fclose(this->_file);
               this->_file = nullptr;
            }
         }
   };
}
