#include "./subrecord_view.h"
#include "./file_view.h"

namespace dovah::tes {
   const record_view& subrecord_view::get_containing_record() const {
      return this->_owner.subviews.record;
   }
   record_view& subrecord_view::get_containing_record() {
      return this->_owner.subviews.record;
   }

   void subrecord_view::read_all_to_string(std::string& dst) {
      dst.assign((const char*)this->data(), this->size());
      this->get_containing_record().skip_bytes(this->size() - this->current_offset());
   }
}