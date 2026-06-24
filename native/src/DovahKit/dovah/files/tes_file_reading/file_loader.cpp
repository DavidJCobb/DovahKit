#include "file_loader.h"
#include "file_threaded_part_loader_base.h"
#include "threads.h"
#include "../../load_order_interfaces/file_load.h"

#include "../../exceptions/file_load_failed.h"
#include "../../notices/file_load_errors/file_header_lists_too_many_dependencies.h"
#include "../../notices/file_load_errors/filesystem_error.h"
#include "../../notices/file_load_errors/interior_cell_block_group_badly_nested.h"
#include "../../notices/file_load_errors/interior_cell_block_has_no_parent_group.h"
#include "../../notices/file_load_errors/malformed_file_header.h"
#include "../../notices/file_load_warnings/record_found_in_wrong_top_level_group.h"

namespace dovah::tes_file_reading {
   file_loader::file_loader(interface_t& intfc) : file_or_file_part_loader(*this, intfc) {
      for (size_t i = 0; i < threads::basic::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::basic(*this));
      }
      for (size_t i = 0; i < threads::dialogue::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::dialogue(*this));
      }
      for (size_t i = 0; i < threads::interior_cell::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::interior_cell(*this));
      }
      for (size_t i = 0; i < threads::worldspace_sub_block::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::worldspace_sub_block(*this));
      }
      for (size_t i = 0; i < threads::worldspace_persistent_cell_children::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::worldspace_persistent_cell_children(*this));
      }
      for (size_t i = 0; i < threads::game_setting::recommended_thread_count; ++i) {
         this->threads.push_back(new threads::game_setting(*this));
      }
   }
   file_loader::~file_loader() {
      if (auto*& p = this->localization_data) {
         delete p;
         p = nullptr;
      }
      for (auto* thread : this->threads) {
         if (!thread)
            continue;
         #if _DEBUG
            if (thread->is_running())
               __debugbreak(); // why are you destroying a (file_loader) when one of its threaded readers is still running?
         #endif
         delete thread;
      }
      this->threads.clear();
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
   void file_loader::_set_filename(const std::filesystem::path& desired, const std::filesystem::path& actual) {
      //
      // The "actual" path is the path the file is loading from, while the "desired" path is where we want 
      // the file to be. When we're loading a new file, these are the same path. However, when we save a 
      // file, we save it to a temporary file and then rename it to the desired path; if the rename fails, 
      // then the desired and actual paths differ.
      //
      // A file's name can influence whether it is treated as a light or master file. For the purposes of 
      // handling that behavior, we should use the file's desired name to track that status.
      //
      this->path = actual;
      //
      this->header.details &= ~(tes_file_header::detail_flag::file_extension_forces_light | tes_file_header::detail_flag::file_extension_forces_master);
      auto ext = desired.extension().string();
      if (_stricmp(ext.data(), "esl") == 0)
         this->header.details |= tes_file_header::detail_flag::file_extension_forces_light | tes_file_header::detail_flag::file_extension_forces_master;
      else if (_stricmp(ext.data(), "esm") == 0)
         this->header.details |= tes_file_header::detail_flag::file_extension_forces_master;
   }
   //
   void file_loader::_start_threads() {
      for (auto* thread : this->threads)
         thread->start();
   }
   void file_loader::_wait_for_threads() {
      for (auto* thread : this->threads)
         thread->wait_for();
   }
   //
   file_threaded_part_loader_base* file_loader::_get_nth_thread_of_type_impl(const std::type_info& ti, size_t n) {
      size_t i = 0;
      for (auto* thread : this->threads) {
         const auto& type = typeid(*thread);
         if (type == ti) {
            if (i++ == n) {
               ++n;
               return thread;
            }
         }
      }
      return nullptr;
   }
   #pragma endregion

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

   bool file_loader::load(const std::filesystem::path& new_path) {
      if (!new_path.empty())
         this->_set_filename(new_path, new_path);
      else if (this->path.empty()) {
         throw dovah::exceptions::file_load_failed(dovah::exceptions::file_load_failed::error_code::no_filename_specified);
      }
      this->_open_mapped_file(this->path);
      //
      if (!this->_load_header()) {
         auto error = std::make_unique<dovah::notices::file_load_errors::malformed_file_header>();
         auto ex    = dovah::exceptions::file_load_failed();
         
         error->filename    = this->get_filename();
         error->file_offset = this->get_position();
         
         ex.details.file_load_error = std::move(error);
         throw ex;
      }
      {
         object_type ot;
         struct {
            uint32_t basic    = 0;
            uint32_t dialogue = 0;
            uint32_t interior_cell   = 0;
            uint32_t world_sub_block = 0;
            uint32_t exterior_cell   = 0;
            uint32_t game_setting    = 0;
         } thread_indices;
         uint32_t last_worldspace_id = 0;
         uint32_t last_world_cell_id = 0;
         int16_t  last_ext_block_x = 0;
         int16_t  last_ext_block_y = 0;
         while (ot = this->next_record_or_group(), ot != object_type::none) {
            //
            // First, let's define some terms:
            //
            //  - Simple form type: A form type that cannot have child forms (i.e. no 
            //    nested GRUPs).
            //
            //  - Top-group: A top-level GRUP.
            //
            // Okay, so here's what we're doing:
            //
            //  - Top-groups of simple form types: Divide these across multiple threads. 
            //    Each thread will create FormStubs for each record in its assigned top-
            //    groups.
            //
            //  - Dialogue topics:
            //
            //     - These are handled by a single loader, which also handles their 
            //       child GRUPs and INFOs.
            //
            //  - Interior cells:
            //
            //     - Divide the interior cell block GRUPs across multiple threads. Each 
            //       thread will handle the CELL records themselves and any GRUPs nested 
            //       under the block GRUPs.
            //
            //  - Worldspace persistent cells:
            //
            //     - Load the cell here.
            //
            //     - Divide all of the cell's child GRUPs across multiple threads. Each 
            //       thread will handle the records nested under those GRUPs.
            //
            //  - Worldspaces:
            //
            //     - Load the worldspace here.
            //
            //     - Divide the worldspace's cell sub-block GRUPs across multiple threads. 
            //       Each thread will handle the CELL records themselves and any GRUPs 
            //       nested under the sub-block GRUPs.
            //
            auto& group = this->get_current_group();
            if (ot == object_type::group) {
               switch (group.header.type) {
                  //
                  // In order to skip the group's contents, call (group.skip()) and then (continue). In order 
                  // to enter the group's contents and parse them, (continue) without skipping the group.
                  //
                  case group::type::world_children:
                     //
                     // Parse direct children of the worldspace (i.e. the persistent cell).
                     //
                     continue;
                  case group::type::cell_children:
                  case group::type::cell_persistent_children:
                  case group::type::cell_temporary_children:
                     assert(last_world_cell_id != 0 && "We should be ignoring these GRUPs when they don't appear after a worldspace's persistent cell!");
                     {
                        auto* loader = this->_get_nth_thread_of_type<threads::worldspace_persistent_cell_children>(thread_indices.exterior_cell);
                        assert(loader);
                        loader->add_group(last_world_cell_id, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::exterior_cell_block:
                     last_ext_block_y = group.header.label & 0xFFFF;
                     last_ext_block_x = group.header.label >> 0x10;
                     continue;
                  case group::type::exterior_cell_sub_block:
                     assert(last_worldspace_id && "Exterior Cell Block GRUP must follow a WRLD record.");
                     {
                        auto* loader = this->_get_nth_thread_of_type<threads::worldspace_sub_block>(thread_indices.world_sub_block);
                        assert(loader);
                        int16_t sub_x = group.header.label >> 0x10;
                        int16_t sub_y = group.header.label & 0xFFFF;
                        loader->add_group(last_worldspace_id, last_ext_block_x, last_ext_block_y, sub_x, sub_y, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::interior_cell_block:
                     {
                        {  // Error-checking.
                           auto parent = group.get_parent();
                           if (!parent) {
                              auto error = std::make_unique<dovah::notices::file_load_errors::interior_cell_block_has_no_parent_group>();
                              auto ex    = dovah::exceptions::file_load_failed();
         
                              error->filename    = this->get_filename();
                              error->file_offset = this->get_position();
         
                              ex.details.file_load_error = std::move(error);
                              throw ex;
                           }
                           if (parent->header.type != group::type::forms_of_type || _byteswap_ulong(parent->header.label) != 'CELL') {
                              auto error = std::make_unique<dovah::notices::file_load_errors::interior_cell_block_group_badly_nested>();
                              auto ex    = dovah::exceptions::file_load_failed();
         
                              error->filename    = this->get_filename();
                              error->file_offset = this->get_position();
         
                              ex.details.file_load_error = std::move(error);
                              throw ex;
                           }
                        }
                        auto* loader = this->_get_nth_thread_of_type<threads::interior_cell>(thread_indices.interior_cell);
                        assert(loader);
                        loader->add_group(group.header.label, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::forms_of_type:
                     last_world_cell_id = 0;
                     last_ext_block_x   = 0;
                     last_ext_block_y   = 0;
                     break;
                  default:
                     group.skip();
                     continue;
               }
               bool is_dialogue = false;
               bool is_gmst     = false;
               switch (_byteswap_ulong(group.header.label)) {
                  case 'CELL': // interior cells are handled by the Interior Cell Block readers.
                  case 'WRLD': // forms handled here; children handled by the worldspace sub-block readers.
                     //
                     // Don't skip the group; we want to read at least some of the content inside of it. 
                     // However, don't assign the group to a simple-reader either.
                     //
                     continue;
                  case 'DIAL':
                     is_dialogue = true;
                     break;
                  case 'GMST':
                     is_gmst = true;
                     break;
               }
               if (is_gmst) {
                  auto* loader = this->_get_nth_thread_of_type<threads::game_setting>(thread_indices.game_setting);
                  assert(loader);
                  loader->add_group(group.pos);
               } else if (is_dialogue) {
                  auto* loader = this->_get_nth_thread_of_type<threads::dialogue>(thread_indices.dialogue);
                  assert(loader);
                  loader->add_group(_byteswap_ulong(group.header.label), group.pos);
               } else {
                  auto* loader = this->_get_nth_thread_of_type<threads::basic>(thread_indices.basic);
                  assert(loader);
                  loader->add_group(_byteswap_ulong(group.header.label), group.pos);
               }
               //
               // Skip the group's actual content; the main thread only cares about locating the groups 
               // themselves and setting their contents up to be parsed on multiple threads.
               //
               group.skip();
               continue;
            }
            if (ot == object_type::record) {
               last_world_cell_id = 0;
               last_ext_block_x = 0;
               last_ext_block_y = 0;
               //
               // We should only hit records when we choose not to skip a group's contents. 
               // We use this to load worldspaces and their persistent cells in advance, so 
               // that we can multithread their contents a little more flexibly. We can give 
               // a worldspace's descendant GRUPs to different workers without those workers 
               // having to care whether the worldspace is loaded; compare to topics and 
               // infos, which share a worker such that the worker has to load topics first 
               // (which is viable in that case because every topic has only one child GRUP, 
               // so multi-threading within a single topic isn't useful).
               //
               auto& record = this->get_current_record();
               form_type formType = form_type_info::signature_to_form_type(record.signature());
               auto* stub = this->make_stub_for_record();
               switch (group.header.type) {
                  case group::type::world_children:
                     this->set_stub_parent(stub, last_worldspace_id);
                     last_world_cell_id = stub->formID;
                     break;
               }
               if (!this->commit_stub(stub)) {
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(*stub);
               if (record.signature() == 'WRLD')
                  last_worldspace_id = stub->formID;
               //
               if (&group == &this->_groups[0]) { // is this a top-level group?
                  uint32_t group_signature = _byteswap_ulong(group.header.label);
                  if (record.signature() != group_signature) { // misplaced record?
                     form_type group_type = form_type_info::signature_to_form_type(group_signature);
                     
                     notices::file_load_warnings::record_found_in_wrong_top_level_group notice;
                     notice.top_level_group_label = group_signature;
                     notice.record = {
                        .local_id  = record.formID(),
                        .global_id = stub->formID,
                        .signature = record.signature(),
                     };
                     //
                     this->get_file_loader().get_load_interface(*this).log_warning(notice);
                  }
               }
            }
         }
      }
      this->_start_threads();
      this->_wait_for_threads();
      {
         const size_t no_offset = this->threads.size();

         size_t earliest_offset = no_offset;
         for (size_t i = 0; i < this->threads.size(); ++i) {
            auto* thread = this->threads[i];
            if (!thread->exception.captured)
               continue;

            if (earliest_offset == no_offset) {
               earliest_offset = i;
            } else {
               if (thread->exception.thrown_at_file_offset < this->threads[earliest_offset]->exception.thrown_at_file_offset)
                  earliest_offset = i;
            }
         }
         if (earliest_offset != no_offset) {
            std::rethrow_exception(this->threads[earliest_offset]->exception.captured);
         }
      }
      //
      return !this->aborted;
   }
   void file_loader::close() {
      this->_wait_for_threads();
      for (auto* thread : this->threads)
         if (thread)
            thread->_on_file_close();
      this->file = {};
      //
      this->file_data = nullptr;
      this->file_size = 0;
      this->loader    = nullptr;
   }
   void file_loader::reopen() {
      this->reopen(this->path);
   }
   void file_loader::reopen(const std::filesystem::path& path_to_open) {
      this->_open_mapped_file(path_to_open);
   }

   void file_loader::_open_mapped_file(const std::filesystem::path& path_to_open) {
      this->file = {};
      this->file.open(path_to_open.c_str());
      if (!this->file) {
         auto error = std::make_unique<dovah::notices::file_load_errors::filesystem_error>();
         auto ex    = dovah::exceptions::file_load_failed();

         error->filename     = this->get_filename();
         error->winapi_error = this->file.get_error();

         this->file = {};

         ex.details.file_load_error = std::move(error);
         throw ex;
      }
      this->adopt(*this); // update our own (basic_reader) access to the file data
   }
   bool file_loader::_load_header() {
      if (this->next_record_or_group() != object_type::record) {
         return false; // Expected TES4 record.
      }
      auto& r = this->get_current_record();
      if (r.signature() != 'TES4') {
         return false; // Expected TES4 record; got something else.
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
      this->options.uses_string_table = this->header.flags & flag::localized_string_table;
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
               if (!subrecord.read(this->header.author)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               break;
            case 'SNAM': // description
               if (!subrecord.read(this->header.description)) {
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
                  if (!subrecord.read(last.master)) {
                     return false; // don't log an error here; caller should catch (return false) and log a catch-all error
                  }
                  if (this->header.masters.size() > 254) {
                     auto error = std::make_unique<dovah::notices::file_load_errors::file_header_lists_too_many_dependencies>();
                     auto ex    = dovah::exceptions::file_load_failed();
         
                     error->filename    = this->get_filename();
                     error->file_offset = this->get_position();
         
                     ex.details.file_load_error = std::move(error);
                     throw ex;
                  }
                  //
                  // TODO: Have the load process fail if this function encounters any unexpected masters.
                  //
               }
               break;
            case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
               if (last_subrecord != 'MAST') {
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
               if (!subrecord.read(this->header.interior_cell_count)) {
                  return false; // don't log an error here; caller should catch (return false) and log a catch-all error
               }
               this->header.details |= detail_flag::has_incc;
               break;
         }
         last_subrecord = subrecord.signature();
      }
      return true;
   }

   void file_loader::adopt(basic_reader& br) const noexcept {
      br.file_data = (const uint8_t*)this->file.data();
      br.file_size = this->file.size();
      br.loader    = const_cast<file_loader*>(this);
   }

   #pragma region Save interface
   void file_loader::save_interface::update_path(const std::filesystem::path& desired, const std::filesystem::path& actual) const noexcept {
      this->wrapped._set_filename(desired, actual);
   }
   #pragma endregion
}