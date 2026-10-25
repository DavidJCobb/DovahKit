#pragma once
#include <filesystem>
#include "lu/files/mapped_file.h"
#include "lu/passkey.h"
#include "dovah/game.h"
#include "./file_app_specific_flags.h"
#include "./file_header.h"
namespace dovah {
   class active_load_order;
}

namespace dovah::tes {
   class file {
      protected:
         std::filesystem::path   _path;
         lu::files::mapped_file  _file;
         file_header             _header;
         game                    _game = game::skyrim_special;
         file_app_specific_flags _app_flags;
         active_load_order*      _active_load_order = nullptr;

      protected:
         void _update_endianness_from_header();

      public:
         file(lu::passkey<active_load_order>, active_load_order&);
         file(enum game g) : _game(g) {}

         constexpr file_app_specific_flags app_flags() const noexcept { return this->_app_flags; }
         constexpr enum game game() const noexcept { return this->_game; }
         constexpr const file_header& header() const noexcept { return this->_header; }
         constexpr file_header& header() noexcept { return this->_header; }
         constexpr const std::filesystem::path& path() const noexcept { return this->_path; }

         constexpr bool is_light() const noexcept {
            return this->_header.flags.has_all_of<file_header_flag::light>() || this->_app_flags.has_all_of<file_app_specific_flag::file_extension_forces_light>();
         }
         constexpr bool is_master() const noexcept {
            return this->_header.flags.has_all_of<file_header_flag::master>() || this->_app_flags.has_all_of<file_app_specific_flag::file_extension_forces_master>();
         }

         constexpr active_load_order* get_active_load_order() const noexcept {
            return this->_active_load_order;
         }

         constexpr const void* data() const noexcept {
            return this->_file.data();
         }
         constexpr const void* data_at(size_t p) const noexcept {
            return this->_file.data_at(p);
         }
         constexpr size_t size() const noexcept {
            return this->_file.size();
         }

         void open(const std::filesystem::path&); // can throw.
         void close(); // intended for use during the save process, with the file then being reopened by the caller upon a successful save

         // These functions reopen the mapped file without actually loading its contents or re-parsing anything from the header. 
         // They're intended for use during the save process. They can throw.
         void reopen();
         void reopen(const std::filesystem::path&); // reopen with an alternate path (e.g. if we couldn't save to the desired filename)
         void reopen(const std::filesystem::path&, enum game); // reopen with alternate path and name (e.g. for cross-game conversions)

      public: // passkeyed
         static file* _make_hardcoded_dummy_file(lu::passkey<active_load_order>, active_load_order&);
         static file* _make_none_stubs_dummy_file(lu::passkey<active_load_order>, active_load_order&);
   };
}