#pragma once
#include <type_traits>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "shader_module.h"

namespace vulkanDK {
   struct pipeline_stage_specialization_info {
      std::vector<std::byte> data;
      std::vector<VkSpecializationMapEntry> fields;

      inline bool empty() const { return this->data.empty(); }

      pipeline_stage_specialization_info() {}
      template<typename T> pipeline_stage_specialization_info(const T& d, const std::vector<VkSpecializationMapEntry>& f) : fields(f) {
         this->data.resize(sizeof(T));
         memcpy(this->data.data(), &d, sizeof(T));
      };

      template<typename... T> requires (!(std::is_pointer_v<T> || std::is_same_v<T, std::nullptr_t>) && ...)
         pipeline_stage_specialization_info(T... values) { // assumes constantIDs starting from 0
         this->data.resize((sizeof(T) + ...));
         this->fields.resize(sizeof...(T));
         //
         uint32_t i = 0;
         uint32_t n = 0;
         auto append = [&i, &n, this]<typename T>(T& v) {
            memcpy(n + this->data.data(), &v, sizeof(T));
            this->fields[i] = {
               .constantID = i,
               .offset     = n,
               .size       = sizeof(T),
            };
            ++i;
            n += sizeof(T);
         };
         (append(values), ...);
      }
   };

   struct pipeline_stage_info {
      shader_module* module = nullptr; // unowned
      const char*    entry_point_name = nullptr; // function in the shader to call
      VkPipelineShaderStageCreateFlags flags = {};
      VkShaderStageFlagBits            stage = {};
      pipeline_stage_specialization_info specialization_info; // can pass parameters to the shader
   };
}
