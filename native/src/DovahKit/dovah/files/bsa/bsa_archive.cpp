#include "bsa_archive.h"
#include <bit>
#include "../../logging.h"
#include "bsa_archived_file.h"
extern "C" {
   #include "../../../zlib/zlib.h"
   #include "../../../lz4/lz4frame.h"
}

#include <immintrin.h>
#include "../../../helpers/cpuinfo.h"

namespace dovah {
   /*static*/ void bsa_archive::normalize_path_or_path_component(std::string& out) {
      static_assert(sizeof(char) == 1);
      if (out.empty())
         return;
      auto*  data = out.data();
      size_t size = out.size();
      size_t i    = 0;
      //
      if (auto& cpu = cobb::cpuinfo::get(); cpu.extension_support.sse_2 && cpu.extension_support.sse_3) {
         auto mb_a = _mm_set1_epi8('A' - 1);
         auto mb_z = _mm_set1_epi8('Z' + 1);
         auto mb_s = _mm_set1_epi8('/');
         auto fill = _mm_set1_epi8(path_separator);
         for (; i + 15 < size; i += 16) {
            auto ma = _mm_loadu_si128((const __m128i*)(data + i));
            //
            //
            // Goal: for each active byte in (mask_a), OR the byte in (ma) by 0x20
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            // Goal: for each forward slash in (ma), convert it to a backslash.
            //
            mask_a = _mm_cmpeq_epi8(ma, mb_s);
            ma = _mm_blendv_epi8(ma, fill, mask_a); // per byte: dst = (mask & 0x80) ? b : a
            //
            _mm_storeu_si128((__m128i*)(data + i), ma);
         }
         if (i + 7 < size) {
            auto ma = _mm_loadl_epi64((const __m128i*)(data + i));
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            mask_a = _mm_cmpeq_epi8(ma, mb_s);
            ma = _mm_blendv_epi8(ma, fill, mask_a); // per byte: dst = (mask & 0x80) ? b : a
            //
            _mm_storel_epi64((__m128i*)(data + i), ma);
            //
            i += 8;
         }
      }
      for (; i < size; ++i) {
         auto c = data[i];
         if (c >= 'A' && c <= 'Z')
            data[i] = c | 0x20;
         else if (c == '/')
            c = '\\';
      }
   }
   /*static*/ void bsa_archive::normalize_path_component(std::string& out) {
      static_assert(sizeof(char) == 1);
      if (out.empty())
         return;
      auto*  data = out.data();
      size_t size = out.size();
      size_t i = 0;
      //
      if (auto& cpu = cobb::cpuinfo::get(); cpu.extension_support.sse_2 && cpu.extension_support.sse_3) {
         auto mb_a = _mm_set1_epi8('A' - 1);
         auto mb_z = _mm_set1_epi8('Z' + 1);
         for (; i + 15 < size; i += 16) {
            auto ma = _mm_loadu_si128((const __m128i*)(data + i));
            //
            //
            // Goal: for each active byte in (mask_a), OR the byte in (ma) by 0x20
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            _mm_storeu_si128((__m128i*)(data + i), ma);
         }
         if (i + 7 < size) {
            auto ma = _mm_loadl_epi64((const __m128i*)(data + i));
            //
            auto mask_a = _mm_cmpgt_epi8(ma, mb_a); // per byte: (a >= 'A') ? 0xFF : 0
            auto mask_z = _mm_cmplt_epi8(ma, mb_z); // per byte: (a <= 'Z') ? 0xFF : 0
            mask_a = _mm_and_si128(mask_a, mask_z); // bitwise-AND
            mask_a = _mm_and_si128(_mm_set1_epi8(0x20), mask_a); // per byte: (a >= 'A' && a <= 'Z') ? 0x20 : 0
            ma = _mm_or_si128(ma, mask_a); // bitwise-OR
            //
            _mm_storel_epi64((__m128i*)(data + i), ma);
            //
            i += 8;
         }
      }
      for (; i < size; ++i) {
         auto c = data[i];
         if (c >= 'A' && c <= 'Z')
            data[i] = c | 0x20;
      }
   }

   #pragma region File reading and loading
   void bsa_archive::_unchecked_read(void* target, size_t size) noexcept {
      memcpy(target, (const uint8_t*)this->mapping.data() + this->stream_position, size);
      this->stream_position += size;
   }
   void bsa_archive::_read(void* target, size_t size) {
      if (this->stream_position + size >= this->mapping.size())
         this->_throw_load_exception<bsa_unexpected_eof_exception>();
      this->_unchecked_read(target, size);
   }
   void bsa_archive::_read_at(void* target, size_t size, uint64_t offset) const {
      if (offset + size >= this->mapping.size()) {
         bsa_unexpected_eof_exception e;
         e.bsa_path        = this->path;
         e.stream_position = offset;
         throw e;
      }
      memcpy(target, (const uint8_t*)this->mapping.data() + offset, size);
   }
   void bsa_archive::_read(std::string& out) {
      this->_read(out, this->mapping.size() - this->stream_position);
   }
   void bsa_archive::_read(std::string& out, size_t max_length) {
      auto*  src  = (const char*)((const uint8_t*)this->mapping.data() + this->stream_position);
      size_t size = strnlen_s(src, max_length);
      out.assign(src, size);
      this->stream_position += size;
      if (size != max_length)
         this->stream_position += sizeof('\0');
   }
   void bsa_archive::_read_non_null_terminated_string(std::string& out, size_t length) {
      out.resize(length);
      this->_read(out.data(), length);
   }
   void bsa_archive::_read(bsa_archive::folder_entry& folder) {
      this->_read(folder.hash);
      uint32_t count = 0;
      this->_read(count);
      if (this->header.version <= bsa::archive_header::version::skyrim_classic) {
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
      this->stream_position = folder.offset;
      if (this->header.flags & bsa::archive_header::flag::include_directory_names) {
         uint8_t length;
         this->_read(length);
         folder.name.resize(length);
         this->_read(folder.name.data(), folder.name.size());
         {
            auto i = folder.name.find('\0');
            if (i != std::string::npos)
               folder.name.resize(i);
         }
         //
         normalize_path_or_path_component(folder.name);
      }
      folder.files.resize(count);
      for(auto& file : folder.files)
         this->_read(file);
      //
      this->stream_position = pos;
   }
   void bsa_archive::_read(bsa_archive::file_entry& file) {
      if (!this->is_in_bounds(sizeof(file.hash) + sizeof(file.size_and_flags) + sizeof(file.offset))) {
         this->_throw_load_exception<bsa_unexpected_eof_exception>();
      }
      this->_unchecked_read(file.hash);
      this->_unchecked_read(file.size_and_flags);
      this->_unchecked_read(file.offset);
      //
      // The null-terminated filenames produced by "include filenames" are significantly faster 
      // than the length-prefixed full file paths produced by "embed filenames." I'm not entirely 
      // sure why, because for the latter, Visual Studio's performance profiler blames almost the 
      // entirety of the slowdown on reading the `length` byte, as if that can explain a 45-second 
      // difference...
      // 
      // For "include filenames," we have to scan each filename for a null terminator. However, 
      // all filenames are grouped in a big blob, so we're doing sequential reads from two parts 
      // of the file rather than jumping all over the place. Moreover, we don't have to skim each 
      // filename for a directory separator, to shear off the full file path. The speedup probably 
      // comes from one of those factors.
      //
      if (this->header.flags & bsa::archive_header::flag::include_filenames) {
         uint64_t start = this->filename_blob_offset + this->last_filename_offset;
         auto pos = this->stream_position;
         this->stream_position = start;
         this->_read(file.name);
         this->last_filename_offset += (this->stream_position - start);
         this->stream_position = pos;
      } else if (this->header.flags & bsa::archive_header::flag::embed_filenames) {
         const size_t size = this->mapping.size();
         const char*  src  = (const char*)this->mapping.data() + file.offset;
         const char*  end  = (const char*)this->mapping.data() + this->mapping.size();
         if (src >= end) [[unlikely]] {
            this->_throw_load_exception<bsa_unexpected_eof_exception>();
         }
         const uint8_t length = *(uint8_t*)src;
         ++src;
         if (src + length >= end) [[unlikely]] {
            this->_throw_load_exception<bsa_unexpected_eof_exception>();
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
      normalize_path_component(file.name);
      //
      if (file.size() + file.offset > this->mapping.size()) {
         file.corrupt = true;
      }
      //
      ++this->current_file_index;
   }
   void bsa_archive::set_path(const std::filesystem::path& path) {
      assert(!this->mapping.data() && "why do you want to change the path after the BSA has been opened?");
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
      if (auto e = this->mapping.get_error()) {
         bsa_winapi_load_exception ex;
         ex.bsa_path = this->path;
         ex.code     = e;
         throw ex;
      }
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
         this->_throw_load_exception<bsa_load_exception>("bad header sentinel");
      }
      if (this->header.folder_offset != 0x24) {
         dovah::logging::print_line("Warning: The BSA we're loading seems to have unknown content between its file header and its folder listing, or its header is longer than we expect.");
      }
      this->needs_endianness_flip = this->header.flags & bsa::archive_header::flag::big_endian;
      if constexpr (std::endian::native == std::endian::big)
         this->needs_endianness_flip = !this->needs_endianness_flip;
      this->filename_blob_offset = this->header.expected_filename_blob_position();
      //
      this->folders.resize(this->header.folder_count);
      for (auto& folder : this->folders)
         this->_read(folder);
   }
   #pragma endregion

   bool bsa_archive::is_open() const noexcept {
      return this->mapping.data() != nullptr;
   }

   #pragma region File lookup and retrieval
   bsa_archived_file* bsa_archive::lookup_file(const bs_hash& folder, const bs_hash& file) const {
      auto* entry = this->lookup_file_info(folder, file);
      if (!entry)
         return nullptr;
      return this->read_contents_of(*entry);
   }
   bsa_archived_file* bsa_archive::lookup_file(const std::string& path_and_name) const {
      if (path_and_name.empty())
         return nullptr;
      
      std::string folder_name;
      std::string file_name;
      split_path_and_filename(path_and_name, folder_name, file_name);
      if (folder_name.empty() || file_name.empty())
         return nullptr; // as of Oblivion, Bethesda's code can't hash empty strings
      
      std::string extension;
      std::string bare_name = file_name;
      size_t      ext_index = file_name.find_last_of('.');
      if (ext_index != std::string::npos) {
         extension = file_name.substr(ext_index);
         bare_name = file_name.substr(0, ext_index);
         if (bare_name.empty())
            return nullptr; // as of Oblivion, Bethesda's code can't hash empty strings
      }
      bs_hash folder = bs_hash(folder_name.c_str(), {});
      bs_hash file   = bs_hash(bare_name.c_str(),   extension);

      return this->lookup_file(folder, file);
   }

   bsa_archived_file* bsa_archive::read_contents_of(const bsa::packed_file_info& file_info) const {
      assert(this->mapping);
      if (file_info.corrupt)
         return nullptr;
      if (!file_info.size())
         return nullptr;
      
      uint64_t offset = file_info.offset;
      assert(offset + file_info.size() <= this->mapping.size());
      if (this->header.flags & bsa::archive_header::flag::embed_filenames) {
         uint8_t length;
         this->_read_at(length, offset);
         offset += sizeof(length) + length;
      }
      
      auto out = std::make_unique<bsa_archived_file>();

      if (!this->packed_file_is_compressed(file_info)) {
         out->shared.data = this->mapping.data_at(offset);
         out->shared.size = file_info.size();
         return out.release();
      }

      auto*  data = this->mapping.data();
      size_t size = this->mapping.size();
      size_t at   = offset;
      //
      uint32_t length;
      if (at + sizeof(length) >= size)
         return nullptr;
      length = *(uint32_t*)((std::intptr_t)data + at);
      //
      uint64_t    input_pos  = offset + sizeof(length);
      const void* input      = (const uint8_t*)data + input_pos;
      uint32_t    input_size = size - 4;
      if (length) {
         out->owned.resize(length);
         if (this->header.version <= bsa::archive_header::version::skyrim_classic) {
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
      return out.release();
   }
   #pragma endregion
}