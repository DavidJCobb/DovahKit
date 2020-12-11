#include "file_part_loader.h"

namespace dovah::tes_file_reading {
   bool file_part_loader::load_record_at(uint32_t pos) {
      this->reset_parse_state();
      this->set_position(pos);
      return this->next_record_or_group() == object_type::record;
   }
   //
   file_part_loader::object_type file_part_loader::next_record_or_group() {
      if (this->last_error.is_defined())
         return object_type::none;
      auto result = basic_reader::next_record_or_group(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }
   bool file_part_loader::next_subrecord() {
      if (this->last_error.is_defined())
         return false;
      auto result = basic_reader::next_subrecord(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }
}