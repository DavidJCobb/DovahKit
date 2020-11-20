#include "file.h"
#include <filesystem>
#include "../../../helpers/strings.h"
#include "../../form_stub.h"
#include "../../logging.h"
#include "../../notice_code_list.h"
#include "localized_string_file.h"

namespace dovah::tes_file_reading {
   file_reader::_readers::_readers(file_reader& owner) : 
      simple{ owner, owner, owner, owner }, // curly braces here are NOT a typo; this is needed to initialize the array
      interior_cell{ owner, owner, owner, owner },
      worldspace{ owner, owner, owner, owner, owner, owner },
      world_cell{ owner, owner },
      complex(owner, true),
      game_setting(owner)
   {}
   void file_reader::_readers::start() {
      this->complex.start();
      for (auto& reader : this->interior_cell)
         reader.start();
      for (auto& reader : this->worldspace)
         reader.start();
      for (auto& reader : this->simple)
         reader.start();
      for (auto& reader : this->world_cell)
         reader.start();
      this->game_setting.start();
   }
   void file_reader::_readers::wait_for() {
      for (auto& reader : this->simple)
         reader.wait_for();
      for (auto& reader : this->interior_cell)
         reader.wait_for();
      for (auto& reader : this->worldspace)
         reader.wait_for();
      for (auto& reader : this->world_cell)
         reader.wait_for();
      this->complex.wait_for();
      this->game_setting.wait_for();
   }
   float file_reader::_readers::assess_progress() const noexcept {
      float   progress = 0.0F;
      uint8_t count    = 0;
      for (auto& reader : this->interior_cell) {
         progress += reader.assess_progress();
         ++count;
      }
      for (auto& reader : this->simple) {
         progress += reader.assess_progress();
         ++count;
      }
      for (auto& reader : this->world_cell) {
         progress += reader.assess_progress();
         ++count;
      }
      for (auto& reader : this->worldspace) {
         progress += reader.assess_progress();
         ++count;
      }
      progress += this->complex.assess_progress();
      ++count;
      progress += this->game_setting.assess_progress();
      ++count;
      return progress / count;
   }

   file_reader::file_reader(file_load_order& lo) : basic_reader(nullptr), load_order(lo),
      readers{ *this }
   {
      this->file = new cobb::mapped_file();
   }
   file_reader::~file_reader() {
      if (this->file) {
         delete this->file;
         this->file = nullptr;
      }
      if (auto& p = this->localization_data) {
         delete p;
         p = nullptr;
      }
   }
   bool file_reader::load_record_at(uint32_t pos) {
      this->resetParseState();
      this->setPos(pos);
      return this->next_record_or_group() == object_type::record;
   }
   bool file_reader::load_record_at(uint32_t pos, basic_reader* reader) { // for FormStub (multi-threaded building of Use Info)
      reader->owner = this;
      reader->file = this->file;
      reader->resetParseState();
      reader->setPos(pos);
      return reader->next_record_or_group() == object_type::record;
   }
   bool file_reader::fetch_record_header(uint32_t pos, tes_file_record_header& out_header, uint32_t& record_decompressed_size) {
      this->resetParseState();
      this->setPos(pos);
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
   //
   bool file_reader::_load_header() {
      if (this->next_record_or_group() != object_type::record) {
         this->error.code       = file_read_error::error_code::malformed_file;
         this->error.file       = this->name;
         this->error.fileOffset = this->getPos();
         this->error.message    = "Expected TES4 record; no record found.";
         return false;
      }
      auto& r = this->get_current_record();
      if (r.signature() != 'TES4') {
         this->error.code       = file_read_error::error_code::malformed_file;
         this->error.file       = this->name;
         this->error.fileOffset = this->getPos();
         this->error.message    = "Expected TES4 record; got something else.";
         return false;
      }
      this->header.flags = r.flags();
      if (this->name.size() > 4) {  // Force flags based on file extension.
         const char* extension = this->name.data() + this->name.size() - 4;
         if (_strnicmp(".esm", extension, 4) == 0) {
            this->header.flags |= flag::master;
         } else if (_strnicmp(".esl", extension, 4) == 0) {
            this->header.flags |= flag::master | flag::light;
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
                  dovah::logging::print_line("Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
               }
               {
                  this->header.masters.emplace_back();
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.to_string(last.master)) {
                     return false; // don't log an error here; caller should catch (return false) and log a catch-all error
                  }
                  if (this->header.masters.size() > 253) {
                     this->error.code       = file_read_error::error_code::malformed_file;
                     this->error.file       = this->name;
                     this->error.fileOffset = this->getPos();
                     this->error.message    = "This file claims to have more than 253 masters.";
                     return false;
                  }
                  //
                  // TODO: Check if the specified master is in the load order. If not, then we need 
                  // to add it to the load order just before this file. Not yet sure how we oughta 
                  // do that.
                  //
                  // Actually, it might be easier to:
                  //
                  //  - Have LoadOrder pre-load the headers of all relevant files, grab masters as 
                  //    necessary, and construct a final load order. TESPluginFile should not be 
                  //    used to get the headers.
                  //
                  //  - Have LoadOrder load that final load order using TESPluginFile, which will 
                  //    lead to this function being called.
                  //
                  //  - Have the load process fail if this function encounters any unexpected 
                  //    masters.
                  //
               }
               break;
            case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
               if (last_subrecord != 'MAST') {
                  this->error.code       = file_read_error::error_code::malformed_file;
                  this->error.file       = this->name;
                  this->error.fileOffset = this->getPos();
                  if (last_subrecord) {
                     char sig[5];
                     dovah::logging::format_signature(last_subrecord, sig);
                     cobb::sprintf(this->error.message, "Unexpected 'DATA' subrecord in the file header following %s.", sig);
                  } else
                     this->error.message = "Unexpected 'DATA' subrecord at the start of the file header.";
                  return false;
               } else {
                  auto& last = *this->header.masters.rbegin();
                  if (!subrecord.read(last.data)) {
                     dovah::logging::print_line("Warning: failed to read the 'DATA' subrecord for master: %s", last.master.c_str());
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
   bool file_reader::_insert_form(uint32_t formID, form_stub* stub) {
      if (this->aborted)
         return false;
      auto result = this->load_order.accept_form_stub(stub);
      switch (result) {
         case file_load_order::form_id_status::missing_master:
         case file_load_order::form_id_status::out_of_bounds:
            this->error.code       = file_read_error::error_code::out_of_bounds_form_id;
            this->error.file       = this->name;
            this->error.fileOffset = this->getPos();
            this->error.formID    = stub->formID;
            if (result == file_load_order::form_id_status::missing_master)
               this->error.message = "This form's ID corresponds to a missing master.";
            else
               this->error.message = "This form ID's load order prefix is out of bounds.";
            this->abort();
            return false;
         case file_load_order::form_id_status::null_is_not_allowed:
            this->error.code       = file_read_error::error_code::out_of_bounds_form_id;
            this->error.file       = this->name;
            this->error.fileOffset = this->getPos();
            this->error.formID     = stub->formID;
            this->error.message    = "A form cannot use xx000000 as its form ID.";
            this->abort();
            return false;
      }
      return true;
   }
   bool file_reader::load(const char* filepath) {
      this->path = filepath;
      this->name = this->path.filename().string();
      if (!this->open_mapped_file())
         return false;
      //
      dovah::logging::print_line("Opened file: %s", this->name.c_str());
      if (!this->_load_header()) {
         this->error.code       = file_read_error::error_code::malformed_file;
         this->error.file       = this->name;
         this->error.fileOffset = this->getPos();
         this->error.message    = "Failed to read the file header.";
         return false;
      }
      dovah::logging::print_line("Read file header.");
      {
         object_type ot;
         uint32_t   which_simple = 0;
         uint32_t   which_intcell = 0;
         uint32_t   which_world = 0;
         uint32_t   which_world_cell = 0;
         uint32_t   last_worldspace_id = 0;
         uint32_t   last_world_cell_id = 0;
         int16_t    last_ext_block_x = 0;
         int16_t    last_ext_block_y = 0;
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
                        auto& loader = this->readers.world_cell[which_world_cell];
                        if (++which_world_cell >= this->readers.world_cell.size())
                           which_world_cell = 0;
                        loader.add_group(last_world_cell_id, group.pos);
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
                        auto& loader = this->readers.worldspace[which_world];
                        if (++which_world >= this->readers.worldspace.size())
                           which_world = 0;
                        int16_t sub_x = group.header.label >> 0x10;
                        int16_t sub_y = group.header.label & 0xFFFF;
                        loader.add_group(last_worldspace_id, last_ext_block_x, last_ext_block_y, sub_x, sub_y, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::interior_cell_block:
                     {
                        {  // Error-checking.
                           auto parent = group.get_parent();
                           int  err = 0;
                           if (!parent)
                              err = 1;
                           else if (parent->header.type != group::type::forms_of_type)
                              err = 2;
                           else if (_byteswap_ulong(parent->header.label) != 'CELL')
                              err = 3;
                           if (err) {
                              this->error.code       = file_read_error::error_code::malformed_file;
                              this->error.file       = this->name;
                              this->error.fileOffset = this->getPos();
                              this->error.message    = "Bad interior-cell-block group nesting. ";
                              switch (err) {
                                 case 1:
                                    this->error.message += "(No parent group.)";
                                    break;
                                 case 2:
                                    this->error.message += "(Parent group is not a top-level group for a form type.)";
                                    break;
                                 case 3:
                                    this->error.message += "(Parent group is not a group for CELL records.)";
                                    break;
                              }
                              this->abort();
                              break;
                           }
                        }
                        auto& loader = this->readers.interior_cell[which_intcell];
                        if (++which_intcell >= this->readers.interior_cell.size())
                           which_intcell = 0;
                        loader.add_group(group.header.label, group.pos);
                     }
                     group.skip();
                     continue;
                  case group::type::forms_of_type:
                     last_world_cell_id = 0;
                     last_ext_block_x = 0;
                     last_ext_block_y = 0;
                     break;
                  default:
                     group.skip();
                     continue;
               }
               bool is_complex = false;
               bool is_gmst    = false;
               switch (_byteswap_ulong(group.header.label)) {
                  case 'CELL': // contents handled by the Interior Cell Block readers.
                  case 'WRLD': // forms handled here; children handled by the worldspace sub-block readers.
                     //
                     // Don't skip the group; we want to read at least some of the content inside of it. 
                     // However, don't assign the group to a simple-reader either.
                     //
                     continue;
                  case 'DIAL':
                     is_complex = true;
                     break;
                  case 'GMST':
                     is_gmst = true;
                     break;
               }
               if (is_gmst) {
                  this->readers.game_setting.add_group(group.pos);
               } else if (is_complex) {
                  this->readers.complex.add_group(_byteswap_ulong(group.header.label), group.pos);
               } else {
                  auto& loader = this->readers.simple[which_simple];
                  if (++which_simple >= this->readers.simple.size())
                     which_simple = 0;
                  loader.add_group(_byteswap_ulong(group.header.label), group.pos);
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
               // We use this to load worldspaces and their persistent/temporary cells in 
               // advance, so that we can multithread their contents a little more flexibly. 
               // We can assign a worldspace's descendant GRUPs to different workers without 
               // those workers having to care whether the worldspace is loaded; compare to 
               // topics and infos, which share a worker such that the worker has to load 
               // topics first (which is viable in that case because every topic has only 
               // one child GRUP, so multi-threading within a single topic isn't useful).
               //
               auto& record = this->get_current_record();
               form_type_t formType = form_type_info::signature_to_form_type(record.signature());
               auto  stub = this->make_stub_for_record(*this);
               stub->groupInfo.type = (int)group.header.type;
               switch (group.header.type) {
                  case group::type::world_children:
                     stub->groupInfo.parentFormID = last_worldspace_id;
                     last_world_cell_id = stub->formID;
                     break;
               }
               if (!this->_insert_form(stub->formID, stub)) { // also normalizes (stub->formID)
                  delete stub;
                  continue;
               }
               this->extract_high_value_subrecords_for_stub(stub);
               if (record.signature() == 'WRLD')
                  last_worldspace_id = stub->formID;
            }
         }
      }
      this->readers.start();
      this->readers.wait_for();
      //
      return !this->aborted;
   }
   void file_reader::abort() noexcept {
      this->aborted = true;
   }
   float file_reader::assess_load_progress() const noexcept {
      return this->readers.assess_progress();
   }
   void file_reader::close() {
      if (this->file) {
         delete this->file;
         this->file = nullptr;
      }
   }
   bool file_reader::open_mapped_file(const char* filepath) {
      if (filepath)
         this->path = filepath;
      if (!this->file)
         this->file = new cobb::mapped_file();
      //
      /*//
      std::wstring foo;
      auto size = MultiByteToWideChar(CP_ACP, 0, this->path.data(), this->path.size(), foo.data(), 0);
      foo.resize(size);
      MultiByteToWideChar(CP_ACP, 0, this->path.data(), this->path.size(), foo.data(), size);
      //*/
      this->file->open(this->path.c_str());
      if (!*this->file) {
         void* message;
         uint32_t size = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            this->file->get_error(),
            LANG_USER_DEFAULT,
            (LPTSTR)&message,
            0,
            nullptr
         );
         this->error.message.clear();
         uint32_t i = 0;
         while (wchar_t c = ((const wchar_t*)message)[i++])
            this->error.message += c;
         LocalFree(message);
         //
         this->error.code = file_read_error::error_code::filesystem_error;
         this->error.file = this->name;
         delete this->file;
         this->file = nullptr;
         return false;
      }
      return true;
   }
   void file_reader::set_path(const std::filesystem::path& fullpath) {
      this->path = fullpath;
      this->name = fullpath.filename().string();
   }
}