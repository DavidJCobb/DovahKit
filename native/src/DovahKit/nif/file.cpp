#include "file.h"
#include "reader.h"
#include "block.h"
#include "blocks/_factory.h"

namespace nifDK {
   void file::read(void* data, size_t size) {
      auto reader = file_reader(this, data, size);
      reader->owner = this;
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
      reader.read_line_string(this->header.format_name);
      // TODO: when do we read copyright info?
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
      reader.read(string_count);
      reader.read(max_string_length);
      all_strings.resize(string_count);
      for (auto& item : all_strings)
         reader.read_prefixed_string<uint32_t>(item);
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
      //
      this->all_blocks.resize(block_count);
      for (size_t i = 0; i < block_count; ++i) {
         auto tni = block_type_indices[i];
         if (tni >= block_type_names.size())
            return; // TODO: throw or something
         const auto& tn = block_type_names[tni];
         this->all_blocks[i] = create_block_of_type(tn);
      }
      size_t base   = reader.position();
      size_t offset = 0;
      for (size_t i = 0; i < block_count; ++i) {
         auto* b = this->all_blocks[i];
         if (!b)
            continue; // wtf?
         auto size = block_sizes[i];
         auto block_reader = file_reader((void*)((std::intptr_t)data + base + offset), size);
         offset += size;
         //
         b->parse(block_reader);
      }
      //
      // TODO: all blocks that aren't children of some other block should be written to the file's "blocks" member
      //
   }
}