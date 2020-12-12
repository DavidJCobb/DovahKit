#include "file_part_loader.h"
#include "file_loader.h"

namespace dovah::tes_file_reading {
   file_part_loader::file_part_loader(file_loader& o) : owner(o) {}

   bool file_part_loader::_ensure_file() {
      if (!this->is_available()) {
         auto& f = this->owner.get_raw_mapped_file();
         this->file_data = (const uint8_t*)f.data();
         this->file_size = f.size();
      }
   }
   void file_part_loader::_on_file_close() {
      this->file_data = nullptr;
      this->file_size = 0;
   }

   bool file_part_loader::load_record_at(uint32_t pos) {
      this->_ensure_file();
      return file_or_file_part_loader::load_record_at(pos);
   }
   //
   file_part_loader::object_type file_part_loader::next_record_or_group() {
      this->_ensure_file();
      return file_or_file_part_loader::next_record_or_group();
   }
   bool file_part_loader::next_subrecord() {
      this->_ensure_file();
      return file_or_file_part_loader::next_subrecord();
   }
}