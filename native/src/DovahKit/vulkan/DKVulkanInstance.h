#pragma once
#include <vector>
#include <QObject>
#include "../helpers/singleton.h"
#include "_vulkan.h"

namespace vulkanDK {
   class physical_device;
   class surface_renderer;
}

class DKVulkanInstance : public QObject, cobb::singleton {
   Q_OBJECT;
   protected:
      DKVulkanInstance();
      ~DKVulkanInstance();

   public:
      static DKVulkanInstance& get() {
         static DKVulkanInstance instance;
         return instance;
      }

      inline bool available() const noexcept { return this->handle != VK_NULL_HANDLE; }
      inline const VkInstance getHandle() const noexcept { return this->handle; }

      inline const std::vector<const vulkanDK::physical_device*>& physicalDevices() const noexcept {
         //
         // Cursed workaround to grant const-access to not only the vector but the items IN the vector:
         //
         using value_type = decltype(this->physical_devices)::value_type;
         using const_type = std::add_pointer_t<std::add_const_t<std::remove_pointer_t<value_type>>>; // std::add_const_t turns (T*) to (T* const); we need (const T*)
         return *((const std::vector<const_type>*)(&this->physical_devices));
      }

   signals:
      void teardownImminent();
      void teardownComplete();
      
   protected:
      VkInstance handle = VK_NULL_HANDLE;
      std::vector<vulkanDK::physical_device*> physical_devices;
      VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;

      static VkDebugUtilsMessengerCreateInfoEXT _get_debug_create_params();
      static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(
         VkDebugUtilsMessageSeverityFlagBitsEXT,
         VkDebugUtilsMessageTypeFlagsEXT,
         const VkDebugUtilsMessengerCallbackDataEXT*,
         void* pUserData
      );
};
