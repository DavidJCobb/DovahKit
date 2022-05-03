#include "surface_renderer_descriptor_group.h"
#include "config/frames_in_flight.h"
#include "exceptions.h"
#include "surface_renderer.h"

namespace vulkanDK {
   #pragma region descriptor_set_layout_group
   void descriptor_set_layout_group::setup_all(surface_renderer& sr) {
      for (auto& layout : this->list) {
         layout.set_device(sr.logical_device);
         layout.setup();
      }
      for (auto& layout : this->shared_layouts.list) {
         layout.set_device(sr.logical_device);
         layout.setup();
      }
   }
   void descriptor_set_layout_group::teardown_all() {
      for (auto& layout : this->list)
         layout.teardown();
      for (auto& layout : this->shared_layouts.list)
         layout.teardown();
   }

   std::vector<VkDescriptorSetLayout> descriptor_set_layout_group::handles() const {
      std::vector<VkDescriptorSetLayout> out(size + this->shared_layouts.list.size());
      for (size_t i = 0; i < size; ++i)
         out[i] = this->list[i].handle;
      for (size_t i = 0; i < this->shared_layouts.list.size(); ++i)
         out[i + size] = this->shared_layouts.list[i].handle;
      return out;
   }
   std::vector<VkDescriptorSetLayout> descriptor_set_layout_group::handles_for_sets() const {
      std::vector<VkDescriptorSetLayout> out(size);
      for (size_t i = 0; i < size; ++i)
         out[i] = this->list[i].handle;
      {
         auto& sl = this->shared_layouts;
         for(size_t i = 0; i < using_set_count_of(sl.compute_cull_frustum); ++i)
            out.push_back(sl.compute_cull_frustum.handle);
         for (size_t i = 0; i < using_set_count_of(sl.compute_cull_caster); ++i)
            out.push_back(sl.compute_cull_caster.handle);
      }
      return out;
   }

   void descriptor_set_layout_group::_needed_pool_sizes_for(std::vector<VkDescriptorPoolSize>& sizes, const descriptor_set_layout& dl, size_t using_set_count) const {
      for (auto& binding : dl.bindings) {
         auto count = binding.count * config::frames_in_flight_count * using_set_count;
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
            .type            = t,
            .descriptorCount = (uint32_t)count,
         });
      }
   }
   std::vector<VkDescriptorPoolSize> descriptor_set_layout_group::needed_pool_sizes() const {
      std::vector<VkDescriptorPoolSize> sizes;
      for (auto& dl : this->list) {
         this->_needed_pool_sizes_for(sizes, dl);
      }
      {
         auto& sl = this->shared_layouts;
         this->_needed_pool_sizes_for(sizes, sl.compute_cull_frustum, using_set_count_of(sl.compute_cull_frustum));
         this->_needed_pool_sizes_for(sizes, sl.compute_cull_caster,  using_set_count_of(sl.compute_cull_caster));
      }
      return sizes;
   }
   std::vector<uint32_t> descriptor_set_layout_group::variable_binding_counts() const {
      std::vector<uint32_t> variable_counts; // one count per set; sets with no variable-length array will ignore their respective count
      for (auto& layout : this->list) {
         variable_counts.push_back(layout.last_binding_variable_length());
      }
      {  // Shared layouts:
         {
            auto& layout = this->shared_layouts.compute_cull_frustum;
            auto  vc     = layout.last_binding_variable_length();
            for (size_t i = 0; i < descriptor_set_layout_group::using_set_count_of(layout); ++i)
               variable_counts.push_back(vc);
         }
         {
            auto& layout = this->shared_layouts.compute_cull_caster;
            auto  vc     = layout.last_binding_variable_length();
            for (size_t i = 0; i < descriptor_set_layout_group::using_set_count_of(layout); ++i)
               variable_counts.push_back(vc);
         }
      }
      return variable_counts;
   }

   size_t descriptor_set_layout_group::total_set_count() const {
      auto& sl = this->shared_layouts;
      return (
         this->list.size()
         + using_set_count_of(sl.compute_cull_frustum)
         + using_set_count_of(sl.compute_cull_caster)
      ) * config::frames_in_flight_count;
   }
   #pragma endregion
   #pragma region descriptor_set_group
   void descriptor_set_group::allocate_all(surface_renderer& sr) {
      auto& dsl = sr.descriptor_set_layouts;
      //
      // Each descriptor set can have a single descriptor binding that acts as a variable-length 
      // array of descriptors. However, we have to provide suitable maximums for these lists via 
      // an extension struct.
      //
      auto layouts = dsl.handles_for_sets();
      auto variable_count_list = dsl.variable_binding_counts();
      assert(layouts.size() == variable_count_list.size());
      auto variable_count_info = VkDescriptorSetVariableDescriptorCountAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
         .descriptorSetCount = (uint32_t)variable_count_list.size(),
         .pDescriptorCounts  = variable_count_list.data(),
      };
      auto alloc_info = VkDescriptorSetAllocateInfo{
         .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .pNext              = &variable_count_info,
         .descriptorPool     = sr.descriptor_pool,
         .descriptorSetCount = (uint32_t)layouts.size(),
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
   void descriptor_set_group::free_all(surface_renderer& sr) {
      auto free_list = [&sr]<typename T>(T& list) {
         if (list[0] != VK_NULL_HANDLE) {
            vkFreeDescriptorSets(sr.logical_device, sr.descriptor_pool, list.size(), list.data());
            for (auto& item : list)
               item = VK_NULL_HANDLE;
         }
      };
      free_list(this->list);
      free_list(this->sharing_sets.list);
   }
   #pragma endregion
}