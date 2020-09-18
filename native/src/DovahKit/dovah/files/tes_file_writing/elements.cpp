#include "elements.h"
#include "file_writer.h"

namespace dovah::tes_file_writing {
   void record::_write(const void* source, uint32_t size) {
      this->data.reserve(this->pos + size);
      void* target = this->data.data() + size;
      memcpy(target, source, size);
      this->pos += size;
   }
   void record::_close_current_subrecord() {
      auto& subrecord = this->get_current_subrecord();
      if (!subrecord.exists())
         return;
      if (subrecord.pos > std::numeric_limits<decltype(tes_file_subrecord_header::size)>::max()) {
         this->data.reserve(16 + subrecord.data.size());
         //
         this->_write(uint32_t('XXXX'));
         this->_write(uint16_t(4));
         this->_write(uint32_t(subrecord.pos));
         //
         this->_write(subrecord.header.signature);
         this->_write(uint16_t(0));
      } else {
         this->data.reserve(6 + subrecord.data.size());
         //
         this->_write(subrecord.header.signature);
         this->_write(uint16_t(subrecord.pos));
      }
      this->_write(subrecord.data);
      //
      subrecord.header.signature = 0;
      subrecord.header.size      = 0;
      subrecord.pos = 0;
      subrecord.data.free();
   }
   subrecord& record::get_current_subrecord() const noexcept {
      return this->owner._subrecord;
   }
   subrecord& record::open_next_subrecord(uint32_t signature) {
      this->_close_current_subrecord();
      auto& subrecord = this->get_current_subrecord();
      subrecord.header.signature = signature;
      return subrecord;
   }
   void record::close() {
      auto& subrecord = this->get_current_subrecord();
      if (subrecord.exists())
         this->_close_current_subrecord();
      //
      this->header.size = this->pos;
      this->owner._write_record();
      this->header.signature = 0;
      this->header.version   = 0;
      this->header.formID    = 0;
      this->header.size      = 0;
      this->pos = 0;
      this->data.free();
   }

   record& subrecord::get_containing_record() const {
      return this->owner._record;
   }
   void subrecord::write(const void* source, uint32_t size) {
      this->data.reserve(this->pos + size);
      void* target = this->data.data() + size;
      memcpy(target, source, size);
      this->pos += size;
   }
   void subrecord::close() {
      this->get_containing_record()._close_current_subrecord();
   }
}