#include "./subrecord_view.h"
#include "./file_view.h"

namespace dovah::tes {
   const record_view& subrecord_view::get_containing_record() const {
      return this->_owner.subviews.record;
   }
   record_view& subrecord_view::get_containing_record() {
      return this->_owner.subviews.record;
   }

   const file& subrecord_view::get_source_file() const {
      return *this->_owner.get_target_file();
   }

   void subrecord_view::read_all_to_string(std::string& dst) {
      const size_t size = this->size();
      dst.assign((const char*)this->data(), size);
      this->seek(size);
      if (size && dst[size - 1] == '\0')
         dst.resize(size - 1);
   }
}