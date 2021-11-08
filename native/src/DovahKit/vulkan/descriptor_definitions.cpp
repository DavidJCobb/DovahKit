#include "descriptor_definitions.h"
#include <stdexcept>

namespace vulkanDK {
   VkDescriptorSetLayoutBinding descriptor_binding::setup_params() const {
      return VkDescriptorSetLayoutBinding{
         .binding            = this->index,
         .descriptorType     = this->type,
         .descriptorCount    = this->count,
         .stageFlags         = this->shader_stages,
         .pImmutableSamplers = this->immutable_samplers,
      };
   }

   #pragma region descriptor_set_layout
   descriptor_set_layout::descriptor_set_layout(VkDevice d) : device(d) {
   }
   descriptor_set_layout::~descriptor_set_layout() {
      this->teardown();
   }

   void descriptor_set_layout::set_device(VkDevice d) {
      this->teardown();
      this->device = d;
   }

   void descriptor_set_layout::setup() {
      auto&    binding_list  = this->bindings;
      uint32_t binding_count = binding_list.size();
      std::vector<VkDescriptorBindingFlags>     flags_list;
      std::vector<VkDescriptorSetLayoutBinding> binding_params;
      //
      flags_list.resize(binding_count);
      binding_params.resize(binding_count);
      for(size_t i = 0; i < binding_count; ++i) {
         binding_params[i] = binding_list[i].setup_params();
         flags_list[i]     = binding_list[i].flags;
      }
      //
      auto flags_info = VkDescriptorSetLayoutBindingFlagsCreateInfo{
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
         .bindingCount  = binding_count,
         .pBindingFlags = flags_list.data(),
      };
      auto layout_info = VkDescriptorSetLayoutCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .pNext        = &flags_info,
         .bindingCount = binding_count,
         .pBindings    = binding_params.data(),
      };
      if (vkCreateDescriptorSetLayout(this->device, &layout_info, nullptr, &this->handle) != VK_SUCCESS) {
         throw std::runtime_error("[descriptor_set_layout::apply] Failed to create descriptor set layout.");
      }
   }
   void descriptor_set_layout::teardown() {
      if (this->handle != VK_NULL_HANDLE) {
         vkDestroyDescriptorSetLayout(this->device, this->handle, nullptr);
         this->handle = VK_NULL_HANDLE;
      }
   }
   #pragma endregion
}