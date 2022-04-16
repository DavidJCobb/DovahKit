#pragma once
#include <map>
#include <string>
#include <vector>
#include <QByteArray>
#include "helpers/generic_reader_ex.h"
#include "_vulkan.h"
#include "spir_v_entry_point.h"

namespace vulkanDK {
   class spir_v_parser {
      protected:
         cobb::generic_reader_ex reader = cobb::generic_reader_ex(nullptr, 0);
         std::endian endianness = std::endian::native;

         std::map<uint32_t, spir_v_entry_point::execution_mode_data> emd;
         
      public:
         bool valid = true;
         //
         std::vector<spir_v_entry_point> entry_points;
         VkShaderStageFlags stages = VK_SHADER_STAGE_ALL;

         void read(const QByteArray&);

      protected:
         void _read_entry_point();
         void _read_execution_model();
         void _read_opcode();
   };
}