#include "archive.h"
#include <bit>
#include "lu/bytestreams/base_bytereader.h"
extern "C" {
   #include "zlib/zlib.h"
   #include "lz4/lz4frame.h"
}
#include "dovah/exceptions/bsa_extract_errors/lz4_error.h"
#include "dovah/exceptions/bsa_parse_errors/incorrect_header_sentinel.h"
#include "dovah/exceptions/bsa_parse_errors/unexpected_eof.h"
#include "dovah/types/asset_file_buffer.h"
#include "dovah/utils/files/bsa/fold_path.h"
#include "dovah/utils/files/bsa/fold_path_component.h"
#include "dovah/utils/files/bsa/path_to_hashes.h"
#include "./bytereader.h"

namespace dovah::bsa {
   #pragma region File reading and loading
      void archive::_read(bytereader& reader, archive::folder_entry& folder) {
         const size_t archive_size = this->mapping.size();

         uint32_t count = 0;
         if (!reader.read(
            folder.hash.value,
            count
         )) {
            throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
         }
         if (this->header.version <= archive_version::skyrim_classic) {
            if (!reader.read(folder.offset)) {
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            }
         } else {
            if (!reader.read(
               folder.padding,
               folder.offset
            )) {
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            }
         }
         folder.offset -= this->header.total_filename_length; // weird that we have to do this, but we do
         //
         auto pos = reader.position();
         if (folder.offset > archive_size) {
            throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
         }
         reader.unchecked_seek(folder.offset);
         if (this->header.flags.has_all_of<archive_header_flag::include_directory_names>()) {
            uint8_t length;
            if (!reader.read(length))
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            folder.name.resize(length);
            if (!reader.read_bytes(folder.name.data(), folder.name.size()))
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            {
               auto i = folder.name.find('\0');
               if (i != std::string::npos)
                  folder.name.resize(i);
            }
            utils::files::bsa::fold_path_component(folder.name);
         }
         folder.files.resize(count);
         for(auto& file : folder.files)
            this->_read(reader, file);
         //
         reader.unchecked_seek(pos);
      }
      void archive::_read(bytereader& reader, archive::file_entry& file) {
         const size_t archive_size = this->mapping.size();
         if (!reader.read(
            file.hash.value,
            file.size_and_flags,
            file.offset
         )) [[unlikely]] {
            throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
         }
         //
         // The null-terminated filenames produced by "include filenames" are significantly faster 
         // than the length-prefixed full file paths produced by "embed filenames." I'm not entirely 
         // sure why.
         // 
         // For "include filenames," we have to scan each filename for a null terminator. However, 
         // all filenames are grouped in a big blob, so we're doing sequential reads from two parts 
         // of the file rather than jumping all over the place. Moreover, we don't have to skim each 
         // filename for a directory separator, to shear off the full file path. The speedup probably 
         // comes from one of those factors.
         //
         if (this->header.flags.has_all_of<archive_header_flag::include_filenames>()) {
            size_t start = this->filename_blob_offset + this->last_filename_offset;
            if (start > archive_size) [[unlikely]] {
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            }
            const size_t bytes_available = this->mapping.size() - start;
            const auto*  src             = (const char*)this->mapping.data_at(start);
            const size_t size            = strnlen_s(src, bytes_available);
            file.name.assign(src, size);

            this->last_filename_offset += size;
            if (size != bytes_available) {
               //
               // If `strnlen_s` didn't stop early, then it will have stopped just before a 
               // null terminator. Advance past that terminator.
               //
               this->last_filename_offset += sizeof('\0');
            }
         } else if (this->header.flags.has_all_of<archive_header_flag::embed_filenames>()) {
            const char* src = (const char*)this->mapping.data() + file.offset;
            const char* end = (const char*)this->mapping.data() + archive_size;
            if (src >= end) [[unlikely]] {
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            }
            const uint8_t length = *(uint8_t*)src;
            ++src;
            if (src + length >= end) [[unlikely]] {
               throw exceptions::bsa_parse_errors::unexpected_eof(this->path, archive_size);
            }
            const auto full_path = std::string_view(src, length);
            //
            // The path we've just read is a full filepath and name. We need to trim it down to just 
            // the filename.
            //
            auto index = full_path.find_last_of("/\\");
            if (index == std::string::npos)
               file.name = full_path;
            else
               file.name = full_path.substr(index + 1);
         }
         utils::files::bsa::fold_path(file.name);

         if (file.size() + file.offset > archive_size) {
            file.corrupt = true;
         }

         ++this->current_file_index;
      }

      void archive::set_path(std::filesystem::path&& path) {
         assert(!this->is_open() && "why do you want to change the path after the BSA has been opened?");
         this->path = std::move(path);
      }
      void archive::set_path(const std::filesystem::path& path) {
         assert(!this->is_open() && "why do you want to change the path after the BSA has been opened?");
         this->path = path;
      }
      void archive::open() {
         if (this->path.empty())
            return;
         this->open(this->path);
      }
      void archive::open(const std::filesystem::path& path) {
         this->path = path;
         this->mapping.open(path.c_str()); // can throw std::system_error
         this->folders.clear();

         bytereader reader;
         reader.set_buffer({ this->mapping.data(), this->mapping.size() });
         if constexpr (std::endian::native != std::endian::little) {
            reader.set_needs_endianness_swap(true);
         }
         if (!reader.read(
            this->header.sentinel,
            this->header.version,
            this->header.folder_offset,
            this->header.flags,
            this->header.folder_count,
            this->header.file_count,
            this->header.total_folder_name_length,
            this->header.total_filename_length,
            this->header.filetypes
         )) {
            this->mapping = {};
            throw exceptions::bsa_parse_errors::unexpected_eof(this->path, this->mapping.size());
         }
         if (this->header.sentinel != 'BSA\0') {
            this->mapping = {};
            throw exceptions::bsa_parse_errors::incorrect_header_sentinel(this->path);
         }
         #if _DEBUG
            if (this->header.folder_offset != 0x24) {
               __debugbreak(); // The BSA we're loading has unknown content between its file header and folder listing, OR its header is longer than we expect.
            }
         #endif
         {
            const bool is_big_endian = this->header.flags.has_all_of<bsa::archive_header::flag::big_endian>();
            reader.set_needs_endianness_swap(is_big_endian != (std::endian::native == std::endian::big));
         }
         this->filename_blob_offset = this->header.expected_filename_blob_position();
      
         this->folders.resize(this->header.folder_count);
         try {
            for (auto& folder : this->folders) {
               this->_read(reader, folder);
            }
         } catch (...) {
            this->mapping = {};
            throw;
         }
      }
   #pragma endregion

   bool archive::is_open() const noexcept {
      return this->mapping.data() != nullptr;
   }

   #pragma region File lookup and retrieval
      asset_file_buffer archive::lookup_file(const bs_hash& folder, const bs_hash& file) const {
         auto* entry = this->lookup_file_info(folder, file);
         if (!entry)
            return {};
         return this->read_contents_of(*entry);
      }
      asset_file_buffer archive::lookup_file(const std::string& path_and_name) const {
         if (path_and_name.empty())
            return {};
         auto opt_hash_pair = utils::files::bsa::path_to_hashes(path_and_name);
         if (!opt_hash_pair.has_value())
            return {};
         auto [folder_hash, file_hash] = opt_hash_pair.value();
         return this->lookup_file(folder_hash, file_hash);
      }

      asset_file_buffer archive::read_contents_of(const packed_file_info& file_info) const {
         assert(this->mapping);
         if (file_info.corrupt)
            return {};
         if (!file_info.size())
            return {};

         const size_t archive_size = this->mapping.size();
      
         uint64_t offset = file_info.offset;
         if (offset + file_info.size() > archive_size) {
            #if _DEBUG
               __debugbreak(); // BSA packed-file-info points past the end of the archive file?
            #endif
            return {};
         }


         if (this->header.flags.has_all_of<archive_header_flag::embed_filenames>()) {
            uint8_t length;
            if (offset >= archive_size)
               return {};
            length = *(const uint8_t*)this->mapping.data_at(offset);
            offset += sizeof(length);
            offset += length;
         }
      

         if (!this->packed_file_is_compressed(file_info)) {
            return asset_file_buffer(lu::memory::buffer_view{ this->mapping.data_at(offset), file_info.size() });
         }

         uint32_t    uncompressed_size;
         const void* src_buffer;
         {
            if (offset + sizeof(uncompressed_size) >= archive_size)
               return {};
            uncompressed_size = *(const uint32_t*)this->mapping.data_at(offset);
            offset += sizeof(uncompressed_size);
         }
         if (uncompressed_size)
            return {};

         src_buffer = this->mapping.data_at(offset);

         size_t src_bytes_remaining = this->mapping.size() - offset;

         lu::memory::basic_buffer out;
         out.resize(uncompressed_size);
         if (this->header.version <= archive_version::skyrim_classic) {
            //
            // Prior to FO4/SSE, BSA files used zlib.
            //
            uint32_t out_size = uncompressed_size;
            uncompress((Bytef*)out.data(), (uLongf*)&out_size, (Bytef*)src_buffer, (uLongf)src_bytes_remaining);
            #if _DEBUG
               if (out_size != uncompressed_size) {
                  __debugbreak(); // Actual decompressed size did not match the expected decompressed size. Expected `out.size()`, got `length`.
               }
            #endif
         } else {
            //
            // As of FO4/SSE, BSA files use LZ4 frames (as opposed to simple LZ4 blocks).
            //
            LZ4F_dctx* context;
            auto status = LZ4F_createDecompressionContext(&context, LZ4F_VERSION);
            if (LZ4F_isError(status)) {
               throw exceptions::bsa_extract_errors::lz4_error(
                  this->path,
                  exceptions::bsa_extract_errors::lz4_error::failure_type::lz4_initialize,
                  status
               );
            } else {
               LZ4F_decompressOptions_t options = { 0, 0, 0, 0 };
               
               size_t src_size         = src_bytes_remaining;
               void*  dst_buffer       = out.data();
               size_t dst_size         = out.size();
               size_t dst_bytes_filled = 0;

               auto result = LZ4F_decompress(context, dst_buffer, &dst_size, src_buffer, &src_size, &options);
               while (result != 0) {
                  if (LZ4F_isError(result)) {
                     LZ4F_freeDecompressionContext(context);
                     throw exceptions::bsa_extract_errors::lz4_error(
                        this->path,
                        exceptions::bsa_extract_errors::lz4_error::failure_type::lz4_decompress,
                        result
                     );
                  }
                  //
                  // The `decompress` function didn't read all of the data. At this point in time, 
                  // `src_size` is the number of bytes consumed, and `dst_size` is the number of 
                  // bytes produced.
                  //
                  dst_bytes_filled += dst_size;
                  src_buffer = (void*)((char*)src_buffer + src_size);
                  src_size   = (src_bytes_remaining -= src_size);
                  dst_buffer = out.data_at(dst_bytes_filled);
                  dst_size   = out.size() - dst_bytes_filled;
                  result     = LZ4F_decompress(context, dst_buffer, &dst_size, src_buffer, &src_size, &options);
               }
            }
            LZ4F_freeDecompressionContext(context);
         }
         return asset_file_buffer(std::move(out));
      }
   #pragma endregion
}