#include "bsa_archive.h"
#include "../../../helpers/endianness.h"
#include "../../logging.h"
#include "bsa_archived_file.h"
extern "C" {
   #include "../../../zlib/zlib.h"
   #include "../../../lz4/lz4frame.h"
}

namespace dovah {
   #pragma region File reading and loading
   void bsa_archive::_read(void* target, size_t size) {
      if (!this->mapping)
         return;
      memcpy(target, (const uint8_t*)this->mapping.data() + this->stream_position, size);
      this->stream_position += size;
   }
   void bsa_archive::_read_at(void* target, size_t size, uint64_t offset) {
      if (!this->mapping)
         return;
      memcpy(target, (const uint8_t*)this->mapping.data() + offset, size);
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
      folder.offset -= this->header.total_filename_length; // weird that we have to do this, but we do
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
         //
         for (auto& c : folder.name)
            c = tolower(c, std::locale());
      }
      for (uint32_t i = 0; i < count; ++i) {
         auto& file = folder.files.emplace_back();
         this->_read(file);
      }
      //
      this->stream_position = pos;
   }
   void bsa_archive::_read(bsa_archive::file_entry& file) {
      this->_read(file.hash);
      this->_read(file.size_and_flags);
      this->_read(file.offset);
      //
      auto pos = this->stream_position;
      if (this->header.flags & bsa_header::flag::embed_filenames) {
         this->stream_position = file.offset;
         //
         uint8_t     length;
         std::string full_path;
         this->_read(length);
         this->_read_non_null_terminated_string(full_path, length);
         //
         // The path we've just read is a full filepath and name. We need to trim it down to just 
         // the filename.
         //
         file.name.reserve(length);
         auto index = full_path.find_last_of("/\\");
         if (index == std::string::npos)
            file.name = full_path;
         else
            file.name = full_path.substr(index + 1);
      } else if (this->header.flags & bsa_header::flag::include_filenames) {
         uint64_t start = this->filename_blob_offset + this->last_filename_offset;
         this->stream_position = start;
         this->_read(file.name);
         this->last_filename_offset += (this->stream_position - start);
      }
      if (!file.name.empty()) {
         for (auto& c : file.name)
            c = tolower(c, std::locale());
      }
      this->stream_position = pos;
      //
      ++this->current_file_index;
   }
   void bsa_archive::set_path(const std::filesystem::path& path) {
      if (this->mapping.data())
         return;
      this->path = path;
   }
   void bsa_archive::open() {
      if (this->path.empty())
         return;
      this->open(this->path);
   }
   void bsa_archive::open(const std::filesystem::path& path) {
      this->path = path;
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
      this->filename_blob_offset = this->header.expected_filename_blob_position();
      //
      for (uint32_t i = 0; i < this->header.folder_count; ++i) {
         auto& folder = this->folders.emplace_back();
         this->_read(folder);
      }
   }
   #pragma endregion

   bool bsa_archive::is_open() const noexcept {
      return this->mapping.data() != nullptr;
   }

   #pragma region File lookup and retrieval
   const bsa_archive::file_entry* bsa_archive::folder_entry::find_file(const bs_hash& file_hash, const std::string& file_name) const noexcept {
      for (auto& file : this->files) {
         if (file.hash > file_hash)
            return nullptr;
         if (file.hash == file_hash) {
            if (!file_name.empty() && !file.name.empty()) {
               if (_stricmp(file_name.data(), file.name.data()) != 0)
                  continue;
            }
            return &file;
         }
      }
      return nullptr;
   }
   const bsa_archive::file_entry* bsa_archive::find_file(const bs_hash& folder_hash, const bs_hash& file_hash, const std::string& folder_name, const std::string& file_name) const noexcept {
      for (auto& folder : this->folders) {
         if (folder.hash > folder_hash)
            return nullptr;
         if (folder.hash == folder_hash) {
            if (!folder_name.empty() && !folder.name.empty()) {
               if (_stricmp(folder_name.data(), folder.name.data()) != 0)
                  continue;
            }
            return folder.find_file(file_hash, file_name);
         }
      }
      return nullptr;
   }
   bsa_archived_file* bsa_archive::retrieve_entry(const bsa_archive::file_entry& entry) {
      auto size = entry.size();
      if (!size)
         return nullptr;
      //
      uint64_t offset = entry.offset;
      if (this->header.flags & bsa_header::flag::embed_filenames) {
         uint8_t length;
         this->_read_at(length, offset);
         offset += length;
      }
      bool compressed = entry.non_default_compression();
      if (this->header.flags & bsa_header::flag::compressed_by_default)
         compressed = !compressed;
      //
      auto out = new bsa_archived_file;
      if (compressed) {
         auto prior = this->stream_position;
         //
         this->stream_position = offset;
         uint32_t length;
         this->_read(length);
         //
         uint64_t    input_pos  = offset + sizeof(length);
         const void* input      = (const uint8_t*)this->mapping.data() + input_pos;
         uint32_t    input_size = size - 4;
         if (length) {
            out->owned.resize(length);
            if (this->header.version <= bsa_header::version::skyrim_classic) {
               //
               // Prior to FO4/SSE, BSA files used zlib.
               //
               uint32_t out_size = length;
               uncompress((Bytef*)out->owned.raw(), (uLongf*)&out_size, (Bytef*)input, input_size);
               if (out_size != length) {
                  dovah::logging::print_line("Size mismatch for zlib-decompressed BSA file with contents at %08X! Expected final size %08X, got size %08X.", input_pos, length, out->owned.size());
               }
            } else {
               //
               // As of FO4/SSE, BSA files use LZ4 frames (as opposed to simple LZ4 blocks).
               //
               LZ4F_dctx* context;
               auto status = LZ4F_createDecompressionContext(&context, LZ4F_VERSION);
               if (LZ4F_isError(status)) {
                  dovah::logging::print_line("LZ4-decompression of a BSA-archived file failed to initialize context; error code %d.", status);
                  out->error = bsa_archived_file::error_code::lz4_error;
                  out->owned.clear();
               } else {
                  LZ4F_decompressOptions_t options = { 0, 0, 0, 0 };
                  //
                  size_t destination_size = out->owned.size();
                  size_t source_size      = input_size;
                  auto result = LZ4F_decompress(context, out->owned.data(), &destination_size, input, &source_size, &options);
                  if (LZ4F_isError(result)) {
                     dovah::logging::print_line("LZ4-decompression of a BSA-archived file failed to initialize context; error code %d.", result);
                     out->error = bsa_archived_file::error_code::lz4_error;
                     out->owned.clear();
                  } else if (result) {
                     dovah::logging::print_line("Size mismatch for LZ4-decompressed BSA file with contents at %08X? Remaining bytecount is roughly %08X.", input_pos, result);
                  }
               }
               LZ4F_freeDecompressionContext(context);
            }
         }
         //
         this->stream_position = prior;
      } else {
         out->shared.data = this->mapping.data_at(offset);
         out->shared.size = size;
      }
      return out;
   }

   bsa_archived_file* bsa_archive::lookup_file(const bs_hash& folder, const bs_hash& file) {
      std::string empty;
      auto* entry = this->find_file(folder, file, empty, empty);
      if (!entry)
         return nullptr;
      return this->retrieve_entry(*entry);
   }
   bsa_archived_file* bsa_archive::lookup_file(const std::string& path_and_name) {
      if (path_and_name.empty())
         return nullptr;
      //
      std::string folder_name;
      std::string file_name;
      std::locale c_locale;
      //
      size_t size = path_and_name.size();
      char   last = '\0';
      for (size_t i = 0; i < size; ++i) {
         auto c = path_and_name[i];
         if (c == '\\' || c == '/') {
            if (file_name.empty()) // skip leading and redundant slashes
               continue;
            if (!folder_name.empty())
               folder_name += '\\';
            folder_name += file_name;
            file_name.clear();
         } else {
            file_name += tolower(c, c_locale);
         }
      }
      if (folder_name.empty() || file_name.empty())
         return nullptr; // as of Oblivion, Bethesda's code can't hash empty strings
      //
      std::string extension;
      std::string bare_name = file_name;
      size_t      ext_index = file_name.find_last_of('.');
      if (ext_index != std::string::npos) {
         extension = file_name.substr(ext_index);
         bare_name = file_name.substr(0, ext_index);
         if (bare_name.empty())
            return nullptr; // as of Oblivion, Bethesda's code can't hash empty strings
      }
      bs_hash folder = bs_hash(folder_name.c_str(), nullptr);
      bs_hash file   = bs_hash(bare_name.c_str(), extension.empty() ? nullptr : extension.c_str());
      auto*   entry  = this->find_file(folder, file, folder_name, file_name);
      if (!entry)
         return nullptr;
      return this->retrieve_entry(*entry);
   }
   #pragma endregion

   bool bsa_archive::for_each_folder(std::function<bool(const folder_entry&)> functor) {
      for (auto& folder : this->folders)
         if (functor(folder))
            return true;
      return false;
   }
   bool bsa_archive::for_each_file_in_folder(const folder_entry& folder, std::function<bool(const folder_entry&, const file_entry&)> functor) {
      for (auto& file : folder.files)
         if (functor(folder, file))
            return true;
      return false;
   }

   bool bsa_archive::file_is_compressed(const file_entry& file) const noexcept {
      bool result = file.non_default_compression();
      if (this->header.flags & bsa_header::flag::compressed_by_default)
         result = !result;
      return result;
   }
}