#pragma once
#include <filesystem>
#include "lu/files/mapped_file.h"
#include "dovah/game.h"
#include "./file_app_specific_flags.h"
#include "./file_header.h"

namespace dovah::tes {
   class file {
      protected:
         std::filesystem::path   _path;
         lu::files::mapped_file  _file;
         file_header             _header;
         game                    _game = game::skyrim_special;
         file_app_specific_flags _app_flags;

      protected:
         void _update_endianness_from_header();

      public:
         file(enum game g) : _game(g) {}

         constexpr file_app_specific_flags app_flags() const noexcept { return this->_app_flags; }
         constexpr enum game game() const noexcept { return this->_game; }
         constexpr const file_header& header() const noexcept { return this->_header; }

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
   };
}