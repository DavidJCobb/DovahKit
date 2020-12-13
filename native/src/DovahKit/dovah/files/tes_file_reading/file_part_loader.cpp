#include "file_part_loader.h"
#include "file_loader.h"

namespace dovah::tes_file_reading {
   file_part_loader::file_part_loader(file_loader& o) : file_or_file_part_loader(o) {}

   void file_part_loader::_on_file_close() {
      this->file_data = nullptr;
      this->file_size = 0;
   }
}