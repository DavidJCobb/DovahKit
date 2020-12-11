#include "file_loader.h"
#include "../../notice_code_list.h"
#include "file_threaded_part_loader_base.h"

namespace dovah::tes_file_reading {
   file_loader::file_loader(interface_t& intfc) : load_interface(intfc) {
      this->threads.reserve(total_threads);
      for (size_t i = 0; i < threads_for_simple_load; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
      for (size_t i = 0; i < threads_for_dialogue_load; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
      for (size_t i = 0; i < threads_for_interior_cell_load; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
      for (size_t i = 0; i < threads_for_worldspace_load; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
      for (size_t i = 0; i < threads_for_worldspace_cell_load; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
      for (size_t i = 0; i < threads_for_game_settings; ++i) {
         file_threaded_part_loader_base* t = static_assert(false, "heap-allocate the threaded reader");
         this->threads.push_back(t);
      }
   }
   file_loader::~file_loader() {
      if (auto*& p = this->localization_data) {
         delete p;
         p = nullptr;
      }
   }

   #pragma region Threading
   float file_loader::assess_load_progress() const noexcept {
      size_t count = 0;
      float  total = 0.0F;
      for (auto* thread : this->threads) {
         if (!thread)
            return NAN;
         ++count;
         total += thread->assess_progress();
      }
      if (!count)
         return NAN;
      return total / count;
   }
   //
   bool file_loader::_start_threads() {
      for (auto* thread : this->threads) {
         assert(thread);
         thread->start();
      }
   }
   bool file_loader::_wait_for_threads() {
      for (auto* thread : this->threads) {
         assert(thread);
         thread->wait_for();
      }
   }
   #pragma endregion

   void file_loader::abort() noexcept {
      this->aborted = true;
   }
   bool file_loader::fetch_record_header(uint32_t pos, tes_file_record_header& out_header, uint32_t& record_decompressed_size) {
      this->reset_parse_state();
      this->set_position(pos);
      out_header = tes_file_record_header();
      if (this->next_record_or_group() == object_type::record) {
         out_header = this->_record.header;
         if (out_header.body_is_compressed()) {
            record_decompressed_size = this->_record.data.size();
         } else {
            record_decompressed_size = out_header.size;
         }
         return true;
      }
      return false;
   }
   std::string file_loader::get_filename() const noexcept {
      return this->path.filename().string();
   }

   bool file_loader::load_record_at(uint32_t pos) {
      this->reset_parse_state();
      this->set_position(pos);
      return this->next_record_or_group() == object_type::record;
   }
   //
   file_loader::object_type file_loader::next_record_or_group() {
      this->_reset_last_error();
      auto result = basic_reader::next_record_or_group(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }
   bool file_loader::next_subrecord() {
      this->_reset_last_error();
      auto result = basic_reader::next_subrecord(); // call super
      if (this->last_error.is_defined())
         this->load_interface.log_load_error(this->last_error);
      return result;
   }

   void file_loader::close() {
      this->abort();
      this->_wait_for_threads();
      for (auto* thread : this->threads)
         if (thread)
            thread->_on_file_close();
      this->file = cobb::mapped_file();
   }

   bool file_loader::_load_header() {
      if (this->next_record_or_group() != object_type::record) {
         detailed_notice error;
         error.code = notice_code::malformed_file; // Expected TES4 record; no record found.
         error.set_cause_file(this->get_filename());
         error.set_file_offset(this->get_position());
         this->load_interface.log_load_error(error);
         return false;
      }
      auto& r = this->get_current_record();
      if (r.signature() != 'TES4') {
         detailed_notice error;
         error.code = notice_code::malformed_file; // Expected TES4 record; got something else.
         error.set_cause_file(this->get_filename());
         error.set_file_offset(this->get_position());
         this->load_interface.log_load_error(error);
         return false;
      }
      {
         auto name = this->get_filename();
         this->header.flags = r.flags();
         if (name.size() > 4) {  // Force flags based on file extension.
            const char* extension = name.data() + name.size() - 4;
            if (_strnicmp(".esm", extension, 4) == 0) {
               this->header.flags |= flag::master;
            } else if (_strnicmp(".esl", extension, 4) == 0) {
               this->header.flags |= flag::master | flag::light;
            }
         }
      }
      this->header.record_version = r.version();
      //
      uint32_t last_subrecord = 0;
      while (auto& subrecord = r.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'HEDR': // required subrecord; TODO: fail if this isn't present
               if (!subrecord.is_in_bounds(12)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               subrecord.unchecked_read(this->header.file_version);
               subrecord.unchecked_read(this->header.record_and_group_count);
               subrecord.unchecked_read(this->header.nextFormID);
               break;
            case 'CNAM': // author/creator
               if (!subrecord.to_string(this->header.author)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               break;
            case 'SNAM': // description
               if (!subrecord.to_string(this->header.description)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               break;
            case 'MAST':
               if (last_subrecord == 'MAST') { // wrong; should be separated by 'DATA'
                  //dovah::logging::print_line("Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
               }
               {
                  this->header.masters.emplace_back();
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.to_string(last.master)) {
                     return false; // don't log an error here; caller should catch (return false) and log a catch-all error
                  }
                  if (this->header.masters.size() > 254) {
                     detailed_notice error;
                     error.code = notice_code::file_has_too_many_dependencies; // Expected TES4 record; no record found.
                     error.set_cause_file(this->get_filename());
                     error.set_file_offset(this->get_position());
                     this->load_interface.log_load_error(error);
                     return false;
                  }
                  //
                  // TODO: Have the load process fail if this function encounters any unexpected masters.
                  //
               }
               break;
            case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
               if (last_subrecord != 'MAST') {
                  detailed_notice error;
                  error.code = notice_code::malformed_file; // Expected TES4 record; no record found.
                  error.set_cause_file(this->get_filename());
                  error.set_file_offset(this->get_position());
                  this->load_interface.log_load_error(error);
                  return false;
               } else {
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.read(last.data)) {
                     //dovah::logging::print_line("Warning: failed to read the 'DATA' subrecord for master: %s", last.master.c_str());
                  }
               }
               break;
            case 'ONAM':
               //
               // TODO
               //
               this->header.details |= detail_flag::has_onam;
               break;
            case 'INTV':
               if (!subrecord.read(this->header.subINTV)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               this->header.details |= detail_flag::has_intv;
               break;
            case 'INCC':
               if (!subrecord.read(this->header.subINCC)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               this->header.details |= detail_flag::has_incc;
               break;
         }
         last_subrecord = subrecord.signature();
      }
      return true;
   }

   void file_loader::log_load_warning(const file_part_loader& from, detailed_notice& n) {
      this->load_interface.log_load_warning(n);
   }
   void file_loader::log_load_error(const file_part_loader& from, detailed_notice& n) {
      this->load_interface.log_load_error(n);
      this->abort();
   }
}