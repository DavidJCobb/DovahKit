#include "_DKVulkanInterface.h"

namespace nifDK::block_interfaces {
   _DKVulkanMeshInterface::~_DKVulkanMeshInterface() {
      this->vulkan_state.mesh_handle = {};
   }
   void _DKVulkanMeshInterface::sever_connection_to_vulkan_mesh(vulkanDK::rendered_mesh_handle m) {
      if (this->vulkan_state.mesh_handle == m)
         this->vulkan_state.mesh_handle = {};
   }
}