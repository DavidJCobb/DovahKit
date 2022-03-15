#include "convert_access_flags_and_pipeline_stages.h"
#include <array>
#include <utility>

namespace {
   constexpr auto any_shader = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
      | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
      | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
      | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
      | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
      | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

   constexpr auto map = std::array<std::pair<VkAccessFlags, VkPipelineStageFlags>, 18>{
      std::pair{
         VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
         VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT
      },
      std::pair{
         VK_ACCESS_INDEX_READ_BIT,
         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
      },
      std::pair{
         VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
         VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
      },
      std::pair{
         VK_ACCESS_UNIFORM_READ_BIT,
         any_shader
      },
      std::pair{
         VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
      },
      std::pair{
         VK_ACCESS_SHADER_READ_BIT,
         any_shader
      },
      std::pair{
         VK_ACCESS_SHADER_WRITE_BIT,
         any_shader
      },
      std::pair{
         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
      },
      std::pair{
         VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
      },
      std::pair{
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
      },
      std::pair{
         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
      },
      std::pair{
         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
      },
      std::pair{
         VK_ACCESS_TRANSFER_READ_BIT,
         VK_PIPELINE_STAGE_TRANSFER_BIT
      },
      std::pair{
         VK_ACCESS_TRANSFER_WRITE_BIT,
         VK_PIPELINE_STAGE_TRANSFER_BIT
      },
      std::pair{
         VK_ACCESS_HOST_READ_BIT,
         VK_PIPELINE_STAGE_HOST_BIT
      },
      std::pair{
         VK_ACCESS_HOST_WRITE_BIT,
         VK_PIPELINE_STAGE_HOST_BIT
      },
      std::pair{
         VK_ACCESS_MEMORY_READ_BIT,
         VkPipelineStageFlags(0)
      },
      std::pair{
         VK_ACCESS_MEMORY_WRITE_BIT,
         VkPipelineStageFlags(0)
      },
   };
}

namespace vulkanDK {
   extern VkPipelineStageFlags access_flags_to_pipeline_stages(VkAccessFlags value, VkPipelineStageFlags allowed_shaders) {
      if (value == 0)
         return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      VkPipelineStageFlags out = 0;
      for (auto& item : map) {
         if (item.first & value) {
            if (item.second == any_shader)
               out |= allowed_shaders;
            else
               out |= item.second;
         }
      }
      return out;
   }
}