#include "bsa_archive.h"
#include "../../../helpers/endianness.h"
#include "../../logging.h"

namespace dovah {
   void bsa_archive::_read(void* target, size_t size) {
      if (!this->mapping)
         return;
      memcpy(target, (const uint8_t*)this->mapping.data() + this->stream_position, size);
   }
   void bsa_archive::_read(std::string& out) {
      char c = 0;
      do {
         this->_read(c);
         if (c)
            out += c;
      } while (c);
   }
   void bsa_archive::_read_non_null_terminated_string(std::string& out, size_t length) {
      out.resize(length);
      this->_read(out.data(), length);
   }
   void bsa_archive::_read(bsa_archive::folder_entry& folder) {
      this->_read(folder.hash);
      uint32_t count = 0;
      this->_read(count);
      if (this->header.version <= bsa_header::version::skyrim_classic) {
         uint32_t offset;
         this->_read(offset);
         folder.offset = offset;
      } else {
         this->_read(folder.padding);
         this->_read(folder.offset);
      }
      //
      auto pos = this->stream_position;
      //
      folder.files.reserve(count);
      this->stream_position = folder.offset;
      if (this->header.flags & bsa_header::flag::include_directory_names) {
         uint8_t length;
         this->_read(length);
         folder.name.reserve(length);
         this->_read(folder.name);
      }
      for (uint32_t i = 0; i < count; ++i) {
         auto& file = folder.files.emplace_back();
         this->_read(file);
      }
      //
      this->stream_position = pos;
   }
   void bsa_archive::_read(file_entry& file) {
      this->_read(file.hash);
      this->_read(file.size_and_flags);
      this->_read(file.offset);
      //
      auto pos = this->stream_position;
      if (this->header.flags & bsa_header::flag::embed_filenames) {
         this->stream_position = file.offset;
         //
         uint8_t length;
         this->_read(length);
         this->_read_non_null_terminated_string(file.name, length);
      }
      this->stream_position = pos;
   }
   void bsa_archive::open(const std::filesystem::path& path) {
      this->mapping.open(path.c_str());
      this->folders.clear();
      this->stream_position = 0;
      //
      this->_read(this->header.sentinel);
      this->_read(this->header.version);
      this->_read(this->header.folder_offset);
      this->_read(this->header.flags);
      this->_read(this->header.folder_count);
      this->_read(this->header.file_count);
      this->_read(this->header.total_folder_name_length);
      this->_read(this->header.total_filename_length);
      this->_read(this->header.filetypes);
      //
      if (this->header.sentinel != _byteswap_ulong('BSA\0')) {
         this->read_error = read_error_code::bad_header_sentinel;
         return;
      }
      if (this->header.folder_offset != 0x24) {
         dovah::logging::print_line("Warning: The BSA we're loading seems to have unknown content between its file header and its folder listing, or its header is longer than we expect.");
      }
      this->needs_endianness_flip = this->header.flags & bsa_header::flag::big_endian;
      if (cobb::endian::native == cobb::endian::big)
         this->needs_endianness_flip = !this->needs_endianness_flip;
      //
      for (uint32_t i = 0; i < this->header.folder_count; ++i) {
         auto& folder = this->folders.emplace_back();
         this->_read(folder);
      }

   }

}