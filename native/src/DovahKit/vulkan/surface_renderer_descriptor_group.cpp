#include "surface_renderer_descriptor_group.h"
#include "exceptions.h"
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region descriptor_set_layout_group
   void descriptor_set_layout_group::setup_all(surface_renderer& sr) {
      for (auto& layout : this->list) {
         layout.set_device(sr.logical_device);
         layout.setup();
      }
      sr.set_debug_object_name(this->standard.handle,   "Descriptor Set Layout: Standard");
      sr.set_debug_object_name(this->fps.handle,        "Descriptor Set Layout: FPS Counter");
      sr.set_debug_object_name(this->world_axes.handle, "Descriptor Set Layout: World Axes Overlay");
   }

   std::vector<VkDescriptorSetLayout> descriptor_set_layout_group::handles() const {
      constexpr size_t size = std::tuple_size_v<decltype(list)>;
      //
      std::vector<VkDescriptorSetLayout> out(size);
      for (size_t i = 0; i < size; ++i)
         out[i] = this->list[i].handle;
      return out;
   }
   std::vector<VkDescriptorPoolSize> descriptor_set_layout_group::needed_pool_sizes(size_t swap_chain_image_count) const {
      std::vector<VkDescriptorPoolSize> sizes;
      for (auto& dl : this->list) {
         for (auto& binding : dl.bindings) {
            auto count = binding.count * swap_chain_image_count;
            //
            auto t    = binding.type;
            bool done = false;
            for(auto& prior : sizes) {
               if (prior.type == t) {
                  prior.descriptorCount += count;
                  done = true;
                  break;
               }
            }
            if (done)
               continue;
            sizes.emplace_back(VkDescriptorPoolSize{
               .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               .descriptorCount = (uint32_t)count,
            });
         }
      }
      return sizes;
   }
   #pragma endregion
   #pragma region descriptor_set_group
   void descriptor_set_group::allocate_all(surface_renderer& sr) {
      auto& dsl = sr.descriptor_set_layouts;
      //
      constexpr size_t size = std::tuple_size_v<decltype(list)>;
      static_assert(size == std::tuple_size_v<decltype(dsl.list)>);
      //
      // Each descriptor set can have a single descriptor binding that acts as a variable-length 
      // array of descriptors. However, we have to provide suitable maximums for these lists via 
      // an extension struct.
      //
      auto layouts = dsl.handles();
      assert(layouts.size() == size);
      std::vector<uint32_t> variable_counts; // one count per set; sets with no variable-length array will ignore their respective count
      for(auto& layout : dsl.list) {
         auto& bl = layout.bindings;
         auto& vc = variable_counts.emplace_back(0);
         for (size_t j = 0; j < bl.size(); ++j) {
            auto& binding = bl[j];
            if (binding.flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
               assert(vc == 0            && "A descriptor set is not allowed to have multiple variable-length descriptor bindings.");
               assert(j == bl.size() - 1 && "If a descriptor set has a variable-length descriptor binding, it must be the last binding in the list.");
               vc = binding.count;
            }
         }
      }
      auto variable_count_info = VkDescriptorSetVariableDescriptorCountAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
         .descriptorSetCount = (uint32_t)variable_counts.size(),
         .pDescriptorCounts  = variable_counts.data(),
      };
      auto alloc_info = VkDescriptorSetAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .pNext              = &variable_count_info,
         .descriptorPool     = sr.descriptor_pool,
         .descriptorSetCount = (uint32_t)size,
         .pSetLayouts        = layouts.data(),
      };
      //
      // WARNING: If the descriptor pool has an inadequate size, vkAllocateDescriptorSets 
      // MAY fail with an VK_ERROR_POOL_OUT_OF_MEMORY error code... However, some device 
      // drivers may try to solve the problem internally instead, which means that that 
      // particular class of error will not fail consistently across all hardware. Beware. 
      //
      auto result = vkAllocateDescriptorSets(sr.logical_device, &alloc_info, this->list.data());
      switch (result) {
         case VK_SUCCESS:
            break;
         case VK_ERROR_OUT_OF_POOL_MEMORY:
            throw result_exception(result, "[vulkanDK::descriptor_set_group::allocate_all] Failed to allocate descriptor sets: descriptor pool is too small.");
         case VK_ERROR_FRAGMENTED_POOL:
            throw result_exception(result, "[vulkanDK::descriptor_set_group::allocate_all] Failed to allocate descriptor sets: descriptor pool is too fragmented.");
         case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw result_exception(result, "[vulkanDK::descriptor_set_group::allocate_all] Failed to allocate descriptor sets: insufficient device memory.");
         case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw result_exception(result, "[vulkanDK::descriptor_set_group::allocate_all] Failed to allocate descriptor sets: insufficient CPU-side memory.");
         default:
            throw result_exception(result, "[vulkanDK::descriptor_set_group::allocate_all] Failed to allocate descriptor sets.");
      }
   }
   #pragma endregion
}