#include "elements.h"
#include "file_writer.h"
#include "../../localized_strings.h"

namespace dovah::tes_file_writing {
   void record::_write_impl(const void* source, uint32_t size) {
      this->data.resize(this->pos + size);
      void* target = this->data.data() + this->pos;
      memcpy(target, source, size);
      this->pos += size;
   }
   void record::_write_impl(const tes_file_subrecord_header& header) {
      this->reserve_more(sizeof(tes_file_subrecord_header));
      this->_write(_byteswap_ulong(header.signature));
      this->_write(header.size);
   }
   void record::_close_current_subrecord() {
      auto& subrecord = this->get_current_subrecord();
      if (!subrecord.exists())
         return;
      if (subrecord.pos > std::numeric_limits<decltype(tes_file_subrecord_header::size)>::max()) {
         this->reserve_more(16 + subrecord.data.size()); // we could do data.reserve_more, but if for some godforsaken reason we ever needed to write mid-buffer then that'd break
         //
         this->_write(uint32_t('XXXX'));
         this->_write(uint16_t(4));
         this->_write(uint32_t(subrecord.pos));
         //
         this->_write(_byteswap_ulong(subrecord.header.signature));
         this->_write(uint16_t(0));
      } else {
         this->reserve_more(6 + subrecord.data.size());
         //
         this->_write(_byteswap_ulong(subrecord.header.signature));
         this->_write(uint16_t(subrecord.pos));
      }
      this->_write(subrecord.data);
      //
      subrecord.header.signature = 0;
      subrecord.header.size      = 0;
      subrecord.pos = 0;
      subrecord.data.clear();
   }
   void record::_clear() {
      auto& subrecord = this->get_current_subrecord();
      if (subrecord.exists())
         this->_close_current_subrecord();
      //
      this->header.signature = 0;
      this->header.version   = 0;
      this->header.formID    = 0;
      this->header.size      = 0;
      this->pos = 0;
      this->data.clear();
   }
   void record::_close() {
      auto& subrecord = this->get_current_subrecord();
      if (subrecord.exists())
         this->_close_current_subrecord();
      //
      this->header.size = this->pos;
      this->owner._write_record();
      this->header.signature = 0;
      this->header.version = 0;
      this->header.formID = 0;
      this->header.size = 0;
      this->pos = 0;
      this->data.clear();
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

   void record::write_formID_subrecord(uint32_t signature, const form_reference_t& ref, bool only_if_non_empty) {
      if (only_if_non_empty && !ref)
         return;
      auto& subrecord = this->open_next_subrecord(signature);
      subrecord.write(ref.formID());
      subrecord.close();
   }
   void record::write_formID_subrecord(uint32_t signature, form_id_t formID) {
      auto& subrecord = this->open_next_subrecord(signature);
      subrecord.write(formID);
      subrecord.close();
   }
   void record::write_string_subrecord(uint32_t signature, const char* s) {
      auto& subrecord = this->open_next_subrecord(signature);
      subrecord.write(s, strlen(s) + 1);
      subrecord.close();
   }
   void record::write_string_subrecord(uint32_t signature, const std::string& s) {
      auto& subrecord = this->open_next_subrecord(signature);
      subrecord.reserve_more(s.size() + 1); // make room for the null terminator
      subrecord.write(s.data(), s.size());
      subrecord.write('\0');
      subrecord.close();
   }

   #pragma region subrecord
   void subrecord::_write_impl(const form_reference_t& ref) {
      this->write(bare_form_id_t(ref.formID()));
   }
   void subrecord::_write_impl(const struct_form_reference_t& ref) {
      this->write(bare_form_id_t(ref.formID()));
      if (this->is_skyrim_special())
         this->write(ref.padding);
   }
   void subrecord::_write_impl(const localized_string& field) {
      if (this->owner.use_string_table) {
         this->write(field.index);
         return;
      }
      this->write(field.value);
   }
   //
   record& subrecord::get_containing_record() const {
      return this->owner._record;
   }
   void subrecord::write(const void* source, uint32_t size) {
      this->data.resize(this->pos + size);
      void* target = this->data.data() + this->pos;
      memcpy(target, source, size);
      this->pos += size;
   }
   void subrecord::skip_bytes(uint32_t size) {
      this->data.resize(this->pos + size);
      void* target = this->data.data() + this->pos;
      memset(target, 0, size);
      this->pos += size;
   }
   void subrecord::close() {
      this->get_containing_record()._close_current_subrecord();
   }
   #pragma endregion
}