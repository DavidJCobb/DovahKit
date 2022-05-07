#include "DKVulkanInstance.h"
#include "physical_device.h"
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
      .apiVersion         = VK_API_VERSION_1_1,
   };
   //
   const auto debug_validation_feature_list = std::array{ VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };
   VkValidationFeaturesEXT debug_validation_features = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .pNext = nullptr,
      .enabledValidationFeatureCount  = (uint32_t)debug_validation_feature_list.size(),
      .pEnabledValidationFeatures     = debug_validation_feature_list.data(),
      .disabledValidationFeatureCount = 0,
      .pDisabledValidationFeatures    = nullptr,
   };
   VkInstanceCreateInfo createInfo = {
      .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext                   = &debug_validation_features,
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
   if constexpr (enable_debug_logging) {
      //
      // We actually set up two debug loggers: one, specified in createInfo above, to catch 90% of errors, and 
      // another here to catch errors that occur specifically when creating or destroying the VkInstance.
      //
      auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->handle, "vkCreateDebugUtilsMessengerEXT");
      if (func == nullptr) {
         qDebug("[DKVulkanInstance] Failed to set up debug logging: failed to look up API: vkCreateDebugUtilsMessengerEXT.");
      } else {
         auto params = _get_debug_create_params();
         auto result = func(this->handle, &params, nullptr, &this->debug_messenger);
         if (result != VK_SUCCESS) {
            qDebug("[DKVulkanInstance] Failed to set up debug logging: API call failed.");
            this->debug_messenger = VK_NULL_HANDLE;
         }
      }
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
         this->physical_devices.push_back(new vulkanDK::physical_device(h));
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
      if (this->debug_messenger != VK_NULL_HANDLE) {
         auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->handle, "vkDestroyDebugUtilsMessengerEXT");
         if (func != nullptr)
            func(this->handle, this->debug_messenger, nullptr);
      }
   }
   {
      auto& list = this->physical_devices;
      for (auto* p : list)
         delete p;
      list.clear();
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
      qDebug("[DKVulkanInstance][Validation Layer] %s", pCallbackData->pMessage);
   }
   if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
      qDebug("[DKVulkanInstance][Info] %s", pCallbackData->pMessage);
   }
   return VK_FALSE;
}