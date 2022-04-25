#include "_DKVulkanInterface.h"
#include "vulkan/rendered_mesh.h"

namespace nifDK::block_interfaces {
   _DKVulkanMeshInterface::~_DKVulkanMeshInterface() {
      if (auto& handle = this->vulkan_state.mesh_handle; !handle.empty()) {
         handle->owning_nif = nullptr;
         handle = {};
      }
   }
   void _DKVulkanMeshInterface::sever_connection_to_vulkan_mesh(vulkanDK::rendered_mesh_handle m) {
      if (this->vulkan_state.mesh_handle == m) {
         if (!m.empty())
            m->owning_nif = nullptr;
         this->vulkan_state.mesh_handle = {};
      }
   }
}