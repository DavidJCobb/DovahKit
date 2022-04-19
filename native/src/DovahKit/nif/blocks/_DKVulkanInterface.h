#pragma once
#include "vulkan/scene_item_handle.h"

namespace nifDK::block_interfaces {
   // By virtually inheriting a subclass of this class, specific NIF blocks can store Vulkan-related 
   // state information as needed for DovahKit and its renderer.
   class _DKVulkanInterface {
      public:
         virtual void sever_connection_to_vulkan_mesh(vulkanDK::rendered_mesh_handle) {}
   };

   // Vulkan renderer interface for NIF blocks that represent a single vulkanDK::rendered_mesh, i.e. 
   // NiTriBasedGeom objects and their Bethesda-specific counterparts.
   class _DKVulkanMeshInterface : public virtual _DKVulkanInterface {
      public:
         struct {
            vulkanDK::rendered_mesh_handle mesh_handle;
         } vulkan_state;

         virtual void sever_connection_to_vulkan_mesh(vulkanDK::rendered_mesh_handle m) override {
            if (this->vulkan_state.mesh_handle == m)
               this->vulkan_state.mesh_handle = {};
         }
   };
}