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
      this->reset_parse_state();
      this->set_position(pos);
      return this->next_record_or_group() == object_type::record;
   }
   //
   file_part_loader::object_type file_part_loader::next_record_or_group() {
      this->_ensure_file();
      this->_reset_last_error();
      auto result = basic_reader::next_record_or_group(); // call super
      if (this->last_error.is_defined())
         this->owner.log_load_error(*this, this->last_error);
      return result;
   }
   bool file_part_loader::next_subrecord() {
      this->_ensure_file();
      this->_reset_last_error();
      auto result = basic_reader::next_subrecord(); // call super
      if (this->last_error.is_defined())
         this->owner.log_load_error(*this, this->last_error);
      return result;
   }
}