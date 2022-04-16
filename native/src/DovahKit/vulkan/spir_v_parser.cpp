#include "spir_v_parser.h"
#include <cstdint>
#include "helpers/byteswap.h"

namespace vulkanDK {
   void spir_v_parser::read(const QByteArray& data) {
      this->reader = cobb::generic_reader_ex(data.data(), data.size());
      //
      try {
         uint32_t dword;
         reader.read(dword);
         std::endian endianness = std::endian::native;
         if (dword != 0x07230203) {
            if (dword == cobb::byteswap(0x07230203)) {
               reader.set_endianness(std::endian::native == std::endian::little ? std::endian::big : std::endian::little);
            } else {
               return;
            }
         }
         uint32_t version;
         uint32_t magic_number;
         uint32_t bound;
         uint32_t reserved;
         reader.read(version);
         reader.read(magic_number);
         reader.read(bound);
         reader.read(reserved);
         this->stages = 0;
         while (!reader.at_end()) {
            this->_read_opcode();
         }
         //
         for (auto& item : this->entry_points) {
            auto it = this->emd.find(item.id);
            if (it != this->emd.end()) {
               item.execution_modes = it->second;
            }
         }
         //
         if (this->stages == 0) {
            //
            // Couldn't identify stages. Assume all.
            //
            this->stages = VK_SHADER_STAGE_ALL;
         }
      } catch (std::runtime_error& e) {
         this->valid  = false;
         this->stages = VK_SHADER_STAGE_ALL;
      }
   }

   void spir_v_parser::_read_entry_point() {
      int32_t execution_model;
      reader.read(execution_model);
      if (execution_model < 0)
         return;
      VkShaderStageFlags stage = 0;
      switch (execution_model) {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
            stage = (1 << execution_model); // this doesn't hold true for all execution model values, but it does for mapping these six to VkShaderStageFlagBits
            break;
         case 6: // kernel
            return;
         default:
            //
            // Extension or invalid.
            //
            return;
      }
      uint32_t id;
      std::string name;
      reader.read(id);
      reader.read_null_terminated_string(name);
      this->entry_points.push_back({
         .name   = name,
         .id     = id,
         .stages = stage,
      });
      this->stages |= stage;
   }
   void spir_v_parser::_read_execution_model() {
      uint32_t id;
      uint32_t mode;
      reader.read(id);
      reader.read(mode);
      switch (mode) {
         case 17: // LocalSize
            {
               auto& item = this->emd[id];
               reader.read(item.LocalSize.x);
               reader.read(item.LocalSize.y);
               reader.read(item.LocalSize.z);
            }
            break;
      }
      return;
   }
   void spir_v_parser::_read_opcode() {
      auto start = reader.position();
      uint32_t header;
      reader.read(header);
      uint16_t opcode     = (uint16_t)header;
      uint16_t word_count = (uint16_t)(header >> 16);
      if (opcode == 15) { // OpEntryPoint
         this->_read_entry_point();
      } else if (opcode == 16) { // OpExecutionMode
         this->_read_execution_model();
      }
      auto end = start + ((uint32_t)word_count * 4);
      reader.skip(end - reader.position());
   }
}