#include "DKVulkanInstance.h"
#include "config/validation_layers.h"

namespace {
   static constexpr bool debug_print_all_extensions = false;

   const std::vector<const char*> desired_validation_layers = {
      "VK_LAYER_KHRONOS_validation"
   };

   static constexpr bool enable_debug_logging = vulkanDK::config::enable_validation_layers;

   std::vector<const char*> get_required_extensions() {
      std::vector<const char*> extensions;
      #if _WIN32
         extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
         extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
      #else
         #error DKVulkanInstance: Need to add the appropriate surface extension for your platform.
      #endif
      if constexpr (vulkanDK::config::enable_validation_layers) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }
      return extensions;
   }
}

DKVulkanInstance::DKVulkanInstance() {
   if constexpr (vulkanDK::config::enable_validation_layers) {
      uint32_t count;
      vkEnumerateInstanceLayerProperties(&count, nullptr);
      std::vector<VkLayerProperties> list(count);
      vkEnumerateInstanceLayerProperties(&count, list.data());
      //
      for (const char* name : desired_validation_layers) {
         bool found = false;
         for (const auto& layer : list) {
            if (strcmp(name, layer.layerName) == 0) {
               found = true;
               break;
            }
         }
         if (!found) {
            qDebug("[DKVulkanInstance] Validation layers are unavailable. Cannot create Vulkan instance.");
            return;
         }
      }
   }
   auto extensions = get_required_extensions();
   //
   VkApplicationInfo appInfo = {
      .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName   = "DovahKit",
      .applicationVersion = VK_MAKE_VERSION(0, 0, 0), // TODO
      .pEngineName        = "No Engine",
      .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion         = VK_API_VERSION_1_0,
   };
   VkInstanceCreateInfo createInfo = {
      .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo        = &appInfo,
      .enabledLayerCount       = 0,
      .enabledExtensionCount   = (uint32_t)extensions.size(),
      .ppEnabledExtensionNames = extensions.data(),
   };
   VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = _get_debug_create_params();
   if constexpr (vulkanDK::config::enable_validation_layers) {
      createInfo.enabledLayerCount   = static_cast<uint32_t>(desired_validation_layers.size());
      createInfo.ppEnabledLayerNames = desired_validation_layers.data();
      createInfo.pNext               = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
   }
   if (vkCreateInstance(&createInfo, nullptr, &this->handle) != VK_SUCCESS) {
      this->handle = VK_NULL_HANDLE;
      qDebug("[DKVulkanInstance] Failed to create Vulkan instance!");
      return;
   }
   //
   // Get physical device info:
   //
   {
      uint32_t count = 0;
      vkEnumeratePhysicalDevices(this->handle, &count, nullptr);
      std::vector<VkPhysicalDevice> devices(count);
      vkEnumeratePhysicalDevices(this->handle, &count, devices.data());
      //
      this->physical_devices.reserve(count);
      for (auto& h : devices) {
         if (h == VK_NULL_HANDLE)
            continue;
         this->physical_devices.push_back(h);
      }
   }
}
DKVulkanInstance::~DKVulkanInstance() {
   if (this->handle == VK_NULL_HANDLE)
      return;
   emit teardownImminent();
   //
   // TODO: Tear down all associated contexts, scenes, etc..
   //
   if constexpr (enable_debug_logging) {
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
      if (func != nullptr)
         func(this->instance, this->debugMessenger, nullptr);
   }
   vkDestroyInstance(this->handle, nullptr);
   this->handle = VK_NULL_HANDLE;
   emit teardownComplete();
}

/*static*/ VkDebugUtilsMessengerCreateInfoEXT DKVulkanInstance::_get_debug_create_params() {
   if constexpr (!enable_debug_logging) {
      return { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
   }
   return {
      .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &_debug_callback,
      .pUserData       = nullptr,
   };
}
/*static*/ VKAPI_ATTR VkBool32 VKAPI_CALL DKVulkanInstance::_debug_callback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
   void* pUserData
) {
   if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      qDebug("[DovahKitVulkanInstance][Validation Layer] %s", pCallbackData->pMessage);
   }
   return VK_FALSE;
}