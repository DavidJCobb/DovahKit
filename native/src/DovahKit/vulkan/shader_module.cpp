#include "shader_module.h"
#include "helpers/byteswap.h"
#include "helpers/generic_reader.h"
#include "exceptions.h"

namespace {
   static constexpr bool double_check_shader_entry_points = true
      #if _DEBUG
         || true
      #endif
   ;
}

namespace {
   void _parse_entry_point(vulkanDK::shader_module& sm, cobb::generic_reader& stream, std::endian endianness) {
      int32_t execution_model;
      if (!stream.read(execution_model))
         return;
      if (endianness != std::endian::native)
         execution_model = cobb::byteswap(execution_model);
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
      std::string name;
      size_t length = 0;
      {
         size_t size = stream.size();
         size_t i    = stream.position();
         for (; i < size; ++i) {
            auto byte = *(const uint8_t*)stream.data_at(i);
            if (byte)
               ++length;
            else
               break;
         }
         if (i == size) {
            stream.skip(size - stream.position()); // skip to end
            return;
         }
      }
      name.resize(length);
      stream.read(name.data(), length);
      //
      sm.data.entry_points.push_back({
         .name   = name,
         .stages = stage,
      });
      sm.data.stages |= stage;
   }
   void _parse_shader(vulkanDK::shader_module& sm, const QByteArray& compiled) {
      auto stream = cobb::generic_reader(compiled.data(), compiled.size());
      //
      uint32_t dword;
      if (!stream.read(dword))
         return;
      std::endian endianness = std::endian::native;
      if (dword != 0x07230203) {
         if (dword == cobb::byteswap(0x07230203)) {
            endianness = std::endian::native == std::endian::little ? std::endian::big : std::endian::little;
         } else {
            return;
         }
      }
      uint32_t version;
      uint32_t magic_number;
      uint32_t bound;
      uint32_t reserved;
      if (!(stream.read(version) && stream.read(magic_number) && stream.read(bound) && stream.read(reserved)))
         return;
      sm.data.stages = 0;
      while (!stream.at_end()) {
         auto start = stream.position();
         uint32_t header;
         if (!stream.read(header))
            break;
         if (endianness != std::endian::native)
            header = cobb::byteswap(header);
         //
         uint16_t opcode     = (uint16_t)header;
         uint16_t word_count = (uint16_t)(header >> 16);
         if (opcode == 15) { // OpEntryPoint
            _parse_entry_point(sm, stream, endianness);
         }
         auto end = start + ((uint32_t)word_count * 4);
         stream.skip(end - stream.position());
      }
      if (sm.data.stages == 0) {
         //
         // Couldn't identify stages. Assume all.
         //
         sm.data.stages = VK_SHADER_STAGE_ALL;
      }
   }
}

namespace vulkanDK {
   shader_module::shader_module(VkDevice d, const QByteArray& compiled) : device(d) {
      if (compiled.isNull())
         return;
      auto create_info = VkShaderModuleCreateInfo{
         .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = (uint32_t)compiled.size(),
         .pCode    = (const uint32_t*)compiled.data(),
      };
      this->device = d;
      if (auto result = vkCreateShaderModule(d, &create_info, nullptr, &this->handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::shader_module] Failed to create shader module.");
      }
      if constexpr (double_check_shader_entry_points) {
         _parse_shader(*this, compiled);
      }
   }
   shader_module::~shader_module() {
      if (this->device != VK_NULL_HANDLE && this->handle != VK_NULL_HANDLE)
         vkDestroyShaderModule(this->device, this->handle, nullptr);
      this->handle = VK_NULL_HANDLE;
      this->device = VK_NULL_HANDLE;
   }
   //
   shader_module::shader_module(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->device, o.device);
      std::swap(this->data,   o.data);
   }
   shader_module& shader_module::operator=(shader_module&& o) noexcept {
      std::swap(this->handle, o.handle);
      std::swap(this->device, o.device);
      std::swap(this->data,   o.data);
      return *this;
   }
}