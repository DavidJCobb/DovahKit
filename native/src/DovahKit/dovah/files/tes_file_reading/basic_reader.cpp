#include "basic_reader.h"
#include "file.h"
#include "../../../helpers/strings.h"
#include "../../detailed_notice.h"
#include "../../form_stub.h"
#include "../../logging.h"
#include "../../notice_code_list.h"
extern "C" {
   #include "../../../zlib/zlib.h"
}

namespace dovah::tes_file_reading {
   #pragma region basic stream operations
   void basic_reader::setPos(uint32_t pos) {
      this->stream_position = pos;
   }
   uint32_t basic_reader::getPos() {
      return this->stream_position;
   }
   void basic_reader::skipBytes(uint32_t count) {
      this->stream_position += count;
   }
   void basic_reader::rewind(uint32_t by) {
      this->setPos(this->getPos() - by);
   }
   bool basic_reader::isEOF() {
      return !this->file->is_in_bounds(this->stream_position, 1);
   }
   bool basic_reader::is_good() {
      return !this->isEOF();
   }
   #pragma endregion

   bool basic_reader::is_skyrim_special() const noexcept {
      auto owner = this->owner;
      if (!owner)
         return false;
      return owner->header.record_version >= 44;
   }
   bool basic_reader::uses_string_table() const noexcept {
      auto* f = this->as_file();
      if (!f)
         return false;
      return f->header.flags & tes_file_flag::localized_string_table;
   }

   namespace {
      void _log_bad_record_signature(basic_reader* reader, uint32_t pos, uint32_t sig, bool isUnknown) {
         auto* file = reader->as_file();
         assert(file && "TESPluginBaseReader should be or have an owning file.");
         //
         detailed_notice error;
         error.code = notice_code::invalid_record_signature;
         error.set_cause_file(file->get_filename());
         error.set_file_offset(pos);
         error.set_cause_signature(sig);
         file->load_interface.log_load_error(error);
         //
         file->abort();
      }
      bool _validate_record_signature(basic_reader* reader, uint32_t signature, uint32_t pos) {
         auto  file = reader->as_file();
         auto& lo   = file->load_order;
         if (lo.is_loading()) {
            if (!lo.queued_load.options.allow_suspicious_record_signatures) {
               if (form_type_info::signature_is_suspicious(signature)) {
                  _log_bad_record_signature(reader, pos, signature, false);
                  //
                  // TODO: This won't necessarily prevent TESPluginFile and its threaded readers from attempting 
                  // to load more of the file. The error still properly gets logged, because it's the first error 
                  // to log (presumably) and because LoadOrder checks whether errors were logged at the end of 
                  // the load process, but we still waste time trying to load the rest of the file and indeed 
                  // the rest of the load order.
                  //
                  // We may want to add a virtual method, logError, to TESPluginBaseReader; then, TESPluginFile 
                  // could override it to log the filename and to abort, while the threaded readers could 
                  // override it to log their owner's filename and abort their owner.
                  //
                  return false;
               }
            }
            if (!lo.queued_load.options.allow_unknown_record_signatures) {
               if (form_type_info::signature_to_form_type(signature) == form_type::none) {
                  _log_bad_record_signature(reader, pos, signature, true);
                  //
                  // TODO: This won't necessarily prevent TESPluginFile and its threaded readers from attempting 
                  // to load more of the file. The error still properly gets logged, because it's the first error 
                  // to log (presumably) and because LoadOrder checks whether errors were logged at the end of 
                  // the load process, but we still waste time trying to load the rest of the file and indeed 
                  // the rest of the load order.
                  //
                  // We may want to add a virtual method, logError, to TESPluginBaseReader; then, TESPluginFile 
                  // could override it to log the filename and to abort, while the threaded readers could 
                  // override it to log their owner's filename and abort their owner.
                  //
                  return false;
               }
            }
         }
         return true;
      }
      void _log_record_allocation_failure(basic_reader* reader, uint32_t pos, uint32_t size, const record& record) {
         auto  file = reader->as_file();
         assert(file && "TESPluginBaseReader should be or have an owning file.");
         //
         detailed_notice error;
         error.code = notice_code::out_of_memory;
         error.set_cause_file(file->get_filename());
         error.set_file_offset(pos);
         error.set_cause_size(size);
         file->load_interface.log_load_error(error);
         //
         file->abort();
      }
   }
   basic_reader::object_type basic_reader::next_record_or_group() {
      auto filename = this->as_file()->get_filename().c_str();
      //
      auto& record = this->_record;
      if (record) {
         /*//
         if (this->getPos() == record.end)
            _DEBUGMSG("Reached the end of record of type %s from %08X to %08X...", FMT_SIGNATURE(record.signature()), record.head_pos, record.end);
         else
            _DEBUGMSG("Skipping record of type %s from %08X to %08X...", FMT_SIGNATURE(record.signature()), record.head_pos, record.end);
         //*/
         this->setPos(record.end);
         record.reset();
      }
      //
      if (!this->file)
         return object_type::none;
      //
      // TODO: What happens if we hit an empty GRUP? Do we properly advance past it?
      //
      // Make sure we properly handle passing the end of a group:
      //
      auto pos = this->getPos();
      for (uint32_t i = 0; i < this->_groups.size(); i++) {
         auto& group = this->_groups[i];
         if (!group)
            break;
         if (pos >= group.end)
            group.reset();
      }
      //
      if (!this->is_good())
         return object_type::none;
      uint32_t signature;
      this->read(signature);
      signature = _byteswap_ulong(signature);
      if (signature == 'GRUP') {
         this->rewind(4);
         //
         auto pos = this->getPos();
         int32_t parent = -1;
         for (uint32_t i = 0; i < this->_groups.size(); i++) {
            auto& group = this->_groups[i];
            if (!group)
               break;
            parent = i;
         }
         assert(parent + 1 < this->_groups.size());
         auto& group = this->_groups[parent + 1];
         group.pos   = this->getPos();
         this->read(group.header);
         group.header.signature = _byteswap_ulong(group.header.signature);
         group.end   = group.pos + group.header.size;
         return object_type::group;
      }
      //
      // else it must be a record
      //
      this->rewind(4);
      //
      record.head_pos = this->getPos();
      if (auto& group = this->get_current_group())
         if (record.head_pos >= group.end)
            return object_type::none;
      this->read(record.header);
      record.header.signature = _byteswap_ulong(record.header.signature);
      record.body_pos = this->getPos();
      record.end = record.body_pos + record.header.size;
      if (!this->is_good())
         return object_type::none;
      if (!_validate_record_signature(this, record.header.signature, record.head_pos)) // also logs the appropriate error
         return object_type::none;
      {
         switch (record.header.signature) {
            case 'CELL':
            case 'DIAL':
            case 'WRLD':
               this->last_potential_group_parent = record.header.formID;
               break;
            default:
               this->last_potential_group_parent = 0;
         }
      }
      if (record.header.body_is_compressed()) {
         uint32_t decompressed_size;
         uint32_t compressed_size = record.header.size - sizeof(decompressed_size);
         this->read(decompressed_size);
         record.data.resize(decompressed_size);
         if (decompressed_size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.data.empty()) {
               _log_record_allocation_failure(this, record.head_pos, decompressed_size, record);
               return object_type::none;
               //
               // TODO: This won't necessarily prevent TESPluginFile and its threaded readers from attempting 
               // to load more of the file. The error still properly gets logged, because it's the first error 
               // to log (presumably) and because LoadOrder checks whether errors were logged at the end of 
               // the load process, but we still waste time trying to load the rest of the file and indeed 
               // the rest of the load order.
               //
               // We may want to add a virtual method, logError, to TESPluginBaseReader; then, TESPluginFile 
               // could override it to log the filename and to abort, while the threaded readers could 
               // override it to log their owner's filename and abort their owner.
               //
            } else {
               auto input_buffer = malloc(compressed_size);
               this->read(input_buffer, compressed_size);
               uint32_t out_size = decompressed_size;
               uncompress((Bytef*)record.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
               free(input_buffer);
               if (out_size != decompressed_size) {
                  dovah::logging::print_line("Size mismatch for decompressed record! Offset %08X, expected final size %08X, got size %08X.", record.head_pos, decompressed_size, out_size);
               }
               assert(out_size == decompressed_size);
            }
         }
      } else {
         record.data.resize(record.header.size);
         if (record.header.size) { // zero-size records are allowed, and would be indistinguishable from allocation failures
            if (record.data.empty()) {
               _log_record_allocation_failure(this, record.head_pos, record.header.size, record);
               //
               // TODO: This won't necessarily prevent TESPluginFile and its threaded readers from attempting 
               // to load more of the file. The error still properly gets logged, because it's the first error 
               // to log (presumably) and because LoadOrder checks whether errors were logged at the end of 
               // the load process, but we still waste time trying to load the rest of the file and indeed 
               // the rest of the load order.
               //
               // We may want to add a virtual method, logError, to TESPluginBaseReader; then, TESPluginFile 
               // could override it to log the filename and to abort, while the threaded readers could 
               // override it to log their owner's filename and abort their owner.
               //
            } else {
               this->read(record.data.raw(), record.header.size);
            }
         }
      }
      record.offset = 0;
      //
      return object_type::record;
   }
   //
   bool basic_reader::next_subrecord(file_read_error* error) {
      auto& r = this->_record;
      if (this->_subrecord.header.signature) {
         this->_record.skip(this->_subrecord.end - this->_record.stream_pos());
         this->_subrecord.header.signature = 0;
      }
      if (!this->_record.is_in_bounds())
         return false;
      uint16_t size;
      this->_record.read(this->_subrecord.header.signature);
      this->_record.read(size);
      this->_subrecord.header.size = size;
      if (this->_subrecord.header.signature == 'XXXX') {
         //
         // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
         // larger than what can be represented with the usual two-byte length.
         //
         if (this->_subrecord.header.size != 4) {
            if (error) {
               error->code       = file_read_error::error_code::malformed_file;
               error->file       = this->as_file()->get_filename().c_str();
               error->fileOffset = this->getPos();
               error->message    = "Extended subrecord with no length.";
               //
               auto& record = this->get_current_record();
               if (record)
                  error->formID = record.formID();
            }
            this->_subrecord.header.signature = 0;
            return false; // ERROR
         }
         static_assert(sizeof(this->_subrecord.header.size) == 4, "XXXX subrecords store a four-byte subrecord length. Alter the struct definition accordingly.");
         this->_record.read(this->_subrecord.header.size); // the contents of the XXXX subrecord are the length
         //
         // Get the next subrecord.
         //
         this->_record.read(this->_subrecord.header.signature);
         this->_record.skip(2);
      }
      this->_subrecord.header.signature = _byteswap_ulong(this->_subrecord.header.signature);
      this->_subrecord.pos = this->_record.body_pos + this->_record.offset;
      this->_subrecord.end = this->_subrecord.pos + size;
      if (!this->is_good() || !this->_record.is_in_bounds())
         return false;
      return true;
   }
   form_stub* basic_reader::make_stub_for_record(file_reader& file) {
      auto& record = this->get_current_record();
      auto  stub   = new form_stub();
      stub->_add_file(file, record.head_pos);
      stub->formID   = record.formID();
      stub->formType = form_type_info::signature_to_form_type(record.signature());
      if (record.header.flags & tes_file_record_header::flag::deleted)
         stub->flags |= form_stub::flag::flagged_as_deleted;
      return stub;
   }
   void basic_reader::extract_high_value_subrecords_for_stub(form_stub* stub) {
      if (form_type_info::lookup(stub->formType).flags & form_type_info::flag::no_editor_id)
         return;
      //
      struct _state {
         _state() = delete;
         enum {
            found_editor_id   = 0x01,
            found_cell_coords = 0x02,
         };
      };
      constexpr int found_all = _state::found_editor_id | _state::found_cell_coords;
      //
      int   state  = 0;
      auto& record = this->get_current_record();
      bool  is_ext = stub->is_exterior_cell();
      if (!is_ext)
         state |= _state::found_cell_coords;
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID':
               state |= _state::found_editor_id;
               subrecord.to_string(stub->editorID);
               break;
            case 'XCLC':
               state |= _state::found_cell_coords;
               subrecord.read(stub->groupInfo.gridX);
               subrecord.read(stub->groupInfo.gridY);
               break;
            default:
               continue;
         }
         if (state == found_all)
            return;
      }
      if (is_ext && !(state & _state::found_cell_coords)) {
         stub->flags |= form_stub::flag::missing_coordinates;
      }
   }

}