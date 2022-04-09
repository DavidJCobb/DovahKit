#include "descriptor_definitions.h"
#include "exceptions.h"

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
      {
         uint32_t vc = 0;
         for (size_t j = 0; j < binding_count; ++j) {
            auto& binding = binding_list[j];
            if (binding.flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
               if (vc != 0)
                  throw std::logic_error("[descriptor_set_layout::setup] A descriptor set is not allowed to have multiple variable-length descriptor bindings.");
               if (j != binding_count - 1)
                  throw std::logic_error("[descriptor_set_layout::setup] If a descriptor set has a variable-length descriptor binding, it must be the last binding in the list.");
               vc = binding.count;
            }
         }
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
      if (auto result = vkCreateDescriptorSetLayout(this->device, &layout_info, nullptr, &this->handle); result != VK_SUCCESS) {
         throw result_exception(result, "[descriptor_set_layout::apply] Failed to create descriptor set layout.");
      }
   }
   void descriptor_set_layout::teardown() {
      if (this->handle != VK_NULL_HANDLE) {
         vkDestroyDescriptorSetLayout(this->device, this->handle, nullptr);
         this->handle = VK_NULL_HANDLE;
      }
   }

   uint32_t descriptor_set_layout::last_binding_variable_length() const {
      if (this->bindings.empty())
         return 0;
      auto& binding = this->bindings.back();
      if (binding.flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
         return binding.count;
      return 0;
   }
   #pragma endregion
}