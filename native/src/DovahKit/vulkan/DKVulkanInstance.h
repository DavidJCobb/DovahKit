#pragma once
#include <vector>
#include <QObject>
#include "_vulkan.h"
#include "physical_device.h"

namespace vulkanDK {
   class context;
   class device;
   class surface;
}

class DKVulkanInstance : public QObject {
   Q_OBJECT;
   public:
      DKVulkanInstance();
      ~DKVulkanInstance();

      inline bool available() const noexcept { return this->handle != VK_NULL_HANDLE; }
      inline const VkInstance getHandle() const noexcept { return this->handle; }

      vulkanDK::device* createLogicalDevice(vulkanDK::physical_device&); // returned device is owned by this instance
      void deleteLogicalDevice(vulkanDK::device&); // deletes the device

      inline const std::vector<vulkanDK::physical_device>& physicalDevices() const noexcept { return this->physical_devices; }

   signals:
      void ready(vulkanDK::context&);

      void teardownImminent();
      void teardownComplete();
      
   protected:
      VkInstance handle = VK_NULL_HANDLE;
      std::vector<vulkanDK::physical_device> physical_devices;
      std::vector<vulkanDK::device*>  logical_devices;
      std::vector<vulkanDK::surface*> surfaces;

      static VkDebugUtilsMessengerCreateInfoEXT _get_debug_create_params();
      static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(
         VkDebugUtilsMessageSeverityFlagBitsEXT,
         VkDebugUtilsMessageTypeFlagsEXT,
         const VkDebugUtilsMessengerCallbackDataEXT*,
         void* pUserData
      );
};
