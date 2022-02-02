#include "file.h"
#include "reader.h"
#include "block.h"
#include "blocks/_factory.h"

namespace nifDK {
   /*static*/ file_version file_version::from_string(const std::string& s) {
      //
      // R"-(\S+ File Format \d+\.\d+\.\d+\.\d+)-"
      //
      bool   space = false;
      size_t size  = s.size();
      size_t i     = 0;
      for (; i < size; ++i) {
         auto c = s[i];
         if (c == ' ') {
            space = true;
            break;
         }
      }
      if (!space)
         return file_version(0); // failed: no space
      //
      if (s.size() - i <= 12)
         return file_version(0); // failed: not enough room for "File Format " and additional content, after the space
      if (s.compare(i, 12, "File Format ") != 0)
         return file_version(0); // failed: "File Format " not present after the space
      i += 12;
      //
      int      index  = 0;
      int      digits = 0;
      int      part   = 0;
      uint32_t out    = 0;
      for (; i < size; ++i) {
         auto c = s[i];
         if (c == '.') {
            if (part > 255)
               return file_version(0); // failed: a version part is too high (e.g. "256.0.0.0")
            out |= part << (0x08 * index);
            //
            ++index;
            part   = 0;
            digits = 0;
            if (index > 3)
               return file_version(0); // failed: too many '.' (e.g. "1.2.3.4.")
            continue;
         }
         if (c >= '0' && c <= '9') {
            ++digits;
            part = (part * 10) + (c - '0');
            continue;
         }
         return file_version(0); // failed: invalid character
      }
      if (index != 3)
         return file_version(0); // failed: wrong number of parts (e.g. "2.1.0")
      return file_version(out);
   }

   void file::read(void* data, size_t size) {
      auto reader = file_reader(this, data, size);
      //
      uint32_t block_count;
      uint16_t block_type_count;
      std::vector<std::string> block_type_names;
      std::vector<uint16_t> block_type_indices;
      std::vector<uint32_t> block_sizes;
      uint32_t string_count;
      uint32_t max_string_length;
      std::vector<std::string> all_strings;
      uint32_t group_count;
      //
      try {
         reader.read_line_string(this->header.format_name);
         {
            auto version = file_version::from_string(this->header.format_name);
            if (version && version <= file_version::from_parts<3, 1, 0, 0>) {
               std::string copyright; // discard
               reader.read_line_string(copyright);
            }
         }
         reader.read(this->header.version);
         reader.read(this->header.endianness);
         reader.read(this->header.user_versions.primary);
         reader.read(block_count);
         reader.read(this->header.user_versions.secondary);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.creator);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.info[0]);
         reader.read_prefixed_string<uint8_t>(this->header.export_data.info[1]);
         //
         reader.read(block_type_count);
         block_type_names.resize(block_type_count);
         for (uint16_t i = 0; i < block_type_count; ++i) {
            reader.read_prefixed_string<uint32_t>(block_type_names[i]);
         }
         //
         block_type_indices.resize(block_count);
         block_sizes.resize(block_count);
         for (auto& item : block_type_indices)
            reader.read(item);
         for (auto& item : block_sizes)
            reader.read(item);
         //
         reader.read_string_table(file_reader::file_passkey());
         //
         reader.read(group_count);
         if (group_count) {
            //
            // TODO
            //
         }
         //
         // End of header!
         //
         this->all_blocks.resize(block_count);
         for (size_t i = 0; i < block_count; ++i) {
            auto tni = block_type_indices[i];
            if (tni >= block_type_names.size())
               return; // TODO: throw or something
            const auto& tn = block_type_names[tni];
            this->all_blocks[i] = create_block_of_type(tn);
         }
         size_t base = reader.position();
         size_t offset = 0;
         for (size_t i = 0; i < block_count; ++i) {
            auto* b = this->all_blocks[i];
            if (!b)
               continue; // wtf?
            const auto& tn = block_type_names[block_type_indices[i]];
            auto guard = reader.enter_block(file_reader::file_passkey(), i, block_sizes[i], tn);
            b->parse(reader);
         }
      } catch (file_reader::read_error& e) {
         //
         // TODO: report and handle the error
         //
      }
      //
      // TODO: all blocks that aren't children of some other block should be written to a list of top-level blocks on the file object
      //
   }
}