#include "./file.h"
#include "lu/ascii/equals_i.h"
#include "lu/integral_character_code.h"
#include "./bytereader.h"
#include "../active_load_order.h"

namespace dovah::tes {
   file::file(lu::passkey<active_load_order>, active_load_order& alo) {
      this->_active_load_order = &alo;
      this->_game = alo.get_current_game();
   }

   void file::_update_endianness_from_header() {
      bytereader reader;
      reader.set_buffer({ this->_file.data(), this->_file.size() });

      lu::four_cc signature;
      if (reader.read(signature) && signature == '4SET') {
         this->_app_flags.set(file_app_specific_flag::is_opposite_endianness);
      } else {
         this->_app_flags.unset(file_app_specific_flag::is_opposite_endianness);
      }
   }

   void file::open(const std::filesystem::path& path) {
      this->_path = path;
      this->_file.open(path);
      {
         this->_app_flags.unset_each_of<file_app_specific_flag::file_extension_forces_light, file_app_specific_flag::file_extension_forces_master>();
         auto ext = path.extension().string();
         if (ext.size() == 4) {
            if (lu::ascii::equals_i(ext, ".esl")) {
               this->_app_flags.set_each_of<file_app_specific_flag::file_extension_forces_light, file_app_specific_flag::file_extension_forces_master>();
            } else if (lu::ascii::equals_i(ext, ".esm")) {
               this->_app_flags.set(file_app_specific_flag::file_extension_forces_master);
            }
         }
      }
      this->_update_endianness_from_header();
   }
   void file::close() {
      this->_file = {};
   }
   void file::reopen() {
      this->reopen(this->_path, this->_game);
   }
   void file::reopen(const std::filesystem::path& path) {
      this->reopen(path, this->_game);
   }
   void file::reopen(const std::filesystem::path& path, enum game g) {
      this->_game = g;
      this->_file = {};
      this->_path = path;
      this->_file.open(path);
   }

   //
   // passkeyed:
   //

   /*static*/ file* file::_make_hardcoded_dummy_file(lu::passkey<active_load_order>, active_load_order& alo) {
      auto* f = new file(alo.get_current_game());
      f->_active_load_order = &alo;
      f->_app_flags.set_each_of<file_app_specific_flag::is_hardcoded_dummy>();
      return f;
   }
   /*static*/ file* file::_make_none_stubs_dummy_file(lu::passkey<active_load_order>, active_load_order& alo) {
      auto* f = new file(alo.get_current_game());
      f->_active_load_order = &alo;
      f->_app_flags.set_each_of<file_app_specific_flag::is_none_stub_dummy>();
      return f;
   }
}