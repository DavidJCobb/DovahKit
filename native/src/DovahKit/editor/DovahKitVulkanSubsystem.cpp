#include "DovahKitVulkanSubsystem.h"
#include <algorithm>
#include <cstdint>
#include <vector>
#include <QFile>
#include <QResource>

namespace {
   static constexpr auto desired_swap_chain_presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;
}

namespace {
   static constexpr bool debug_print_all_extensions = false;

   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };

   const std::vector<const char*> desired_validation_layers = {
      "VK_LAYER_KHRONOS_validation"
   };

   static constexpr bool enable_validation_layers = true;
   static constexpr bool enable_debug_logging     = enable_validation_layers;

   std::vector<const char*> get_required_extensions() {
      std::vector<const char*> extensions;
      #if _WIN32
         extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
         extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
      #else
         #error Need to add the appropriate surface extension for your platform.
      #endif
      if constexpr (enable_validation_layers) {
         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }
      return extensions;
   }
}

#pragma region QueueFamilies
DovahKitVulkanSubsystem::QueueFamilies::QueueFamilies(VkPhysicalDevice device) {
   auto& subsystem = DovahKitVulkanSubsystem::get();
   //
   uint32_t count = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
   std::vector<VkQueueFamilyProperties> list(count);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &count, list.data());
   //
   for (size_t i = 0; i < count; ++i) {
      const auto& family = list[i];
      //
      if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
         this->set(this->families.graphics, i);
      }
      {  // Can this device render to our render window widget?
         VkBool32 support = false;
         vkGetPhysicalDeviceSurfaceSupportKHR(device, i, subsystem.renderWindowSurface(), &support);
         if (support) {
            this->set(this->families.presentation, i);
         }
      }
      if (this->mask == all_mask_bits_set) // early out; device supports all desired queue family types
         break;
   }
}
void DovahKitVulkanSubsystem::QueueFamilies::set(queue_index_t& entry, queue_index_t value) {
   entry = value;
   //
   auto base = (std::intptr_t)families.list.data();
   auto item = (std::intptr_t)&entry;
   item -= base;
   item /= sizeof(queue_index_t);
   //
   this->mask |= (1 << item);
}
bool DovahKitVulkanSubsystem::QueueFamilies::has(const queue_index_t& entry) const noexcept {
   auto base = (std::intptr_t)families.list.data();
   auto item = (std::intptr_t)&entry;
   assert(item >= base && item < (base + families.list.size() * sizeof(queue_index_t))); // Ensure (entry) is actually an entry in our list.
   item -= base;
   item /= sizeof(queue_index_t);
   return (mask & (1 << item)) != 0;
}
#pragma endregion

#pragma region swap_chain_support_info
DovahKitVulkanSubsystem::swap_chain_support_info::swap_chain_support_info(VkPhysicalDevice device, VkSurfaceKHR surface) {
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &this->capabilities);
   {
      uint32_t count;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
      if (count != 0) {
         this->formats.resize(count);
         vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, this->formats.data());
      }
   }
   {
      uint32_t count;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
      if (count != 0) {
         this->presentation_modes.resize(count);
         vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, this->presentation_modes.data());
      }
   }
}
#pragma endregion

DovahKitVulkanSubsystem::DovahKitVulkanSubsystem() {
   auto& rw = this->surfaces.render_window;
   rw.widget = new QWidget;
   rw.widget->winId(); // force the widget to have a unique HWND
   //
   // Do not call (initialize) here. There are some cases where we need to re-access the 
   // singleton via its getter, but that breaks if the singleton is still being constructed.
   //
}
DovahKitVulkanSubsystem::~DovahKitVulkanSubsystem() {
   this->teardown();
   //
   delete this->surfaces.render_window.widget;
}

void DovahKitVulkanSubsystem::initialize() {
   if (this->initialized)
      return;
   this->initialized = true;
   qDebug("[DovahKitVulkanSubsystem] Initializing...");
   //
   this->setupInstance();
   this->setupDebugMessenger();
   this->setupRenderWindowSurface();
   this->setupPhysicalDevice();
   this->setupLogicalDevice();
   this->setupSwapChain();
   this->setupImageViews();
   this->setupRenderPass();
   this->setupGraphicsPipeline();
   //
   qDebug("[DovahKitVulkanSubsystem] Initialized.");
   emit this->ready();
}
void DovahKitVulkanSubsystem::teardown() {
   if (!this->initialized)
      return;
   //
   emit this->teardownImminent();
   //
   vkDeviceWaitIdle(this->devices.logical); // wait for all draw commands to finish (remember: they're asynch)
   //
   // TODO: Ensure all child objects belonging to the instance are destroyed first.
   //
   vkDestroyPipeline(this->devices.logical, this->pipeline, nullptr);
   vkDestroyPipelineLayout(this->devices.logical, this->pipeline_layout, nullptr);
   vkDestroyRenderPass(this->devices.logical, this->render_pass, nullptr);
   for (auto view : this->swap_chain.views) {
      vkDestroyImageView(this->devices.logical, view, nullptr);
   }
   vkDestroySwapchainKHR(this->devices.logical, this->swap_chain.handle, nullptr);
   vkDestroyDevice(this->devices.logical, nullptr);
   if constexpr (enable_debug_logging) {
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
      if (func != nullptr)
         func(this->instance, this->debugMessenger, nullptr);
   }
   vkDestroySurfaceKHR(this->instance, this->surfaces.render_window.surface, nullptr);
   vkDestroyInstance(this->instance, nullptr);
   //
   emit this->teardownComplete();
}

int32_t DovahKitVulkanSubsystem::deviceScore(VkPhysicalDevice device) const {
   int32_t score  = 0;
   //
   {  // Check for extension support
      uint32_t count;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
      std::vector<VkExtensionProperties> availableExtensions(count);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());
      //
      bool missing = false;
      for (auto& name : device_extensions) {
         bool found = false;
         for (auto& extension : availableExtensions) {
            if (strcmp(extension.extensionName, name) == 0) {
               found = true;
               break;
            }
         }
         if (!found) {
            missing = true;
            break;
         }
      }
      if (missing)
         return 0;
   }
   {  // Check swap chain support
      //
      // From the Vulkan tutorial: "It is important that we only try to query for swap chain 
      // support after verifying that the extension is available." I don't recall if they 
      // ever gave an explanation as to why?
      //
      auto deets = swap_chain_support_info(device, this->surfaces.render_window.surface);
      if (deets.formats.empty())
         return 0;
      if (deets.presentation_modes.empty())
         return 0;
   }
   {  // Check for queue support
      auto f = QueueFamilies(device);
      if (!f.has(f.families.graphics)) // require this queue family type
         return 0;
      if (!f.has(f.families.presentation)) // require this queue family type
         return 0;
   }
   //
   VkPhysicalDeviceProperties properties;
   VkPhysicalDeviceFeatures   features;
   vkGetPhysicalDeviceProperties(device, &properties);
   vkGetPhysicalDeviceFeatures  (device, &features);
   //
   if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { // dedicated graphics card (i.e. not integrated graphics)
      score += 10000;
   }
   score += (std::min)((uint32_t)8192, properties.limits.maxImageDimension2D); // max texture size
   //
   return score;
}

VkShaderModule DovahKitVulkanSubsystem::createShaderModule(const QByteArray compiled_shader) {
   auto create_info = VkShaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (uint32_t)compiled_shader.size(),
      .pCode    = (const uint32_t*)compiled_shader.data(),
   };
   VkShaderModule shaderModule;
   if (vkCreateShaderModule(this->devices.logical, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create shader module.");
   }
   return shaderModule;
}

void DovahKitVulkanSubsystem::setupInstance() {
   if constexpr (enable_validation_layers) {
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
            throw std::runtime_error("[DovahKitVulkanSubsystem] Validation layers are unavailable!");
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
   if constexpr (enable_validation_layers) {
      createInfo.enabledLayerCount   = static_cast<uint32_t>(desired_validation_layers.size());
      createInfo.ppEnabledLayerNames = desired_validation_layers.data();
      createInfo.pNext               = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
   }
   if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
      this->failed = true;
      qDebug("[DovahKitVulkanSubsystem] Failed to create Vulkan instance!");
      return;
   }
}
void DovahKitVulkanSubsystem::setupDebugMessenger() {
   if constexpr (enable_debug_logging) {
      //
      // We actually set up two debug loggers: one, specified in createInfo above, to catch 90% of errors, and 
      // another here to catch errors that occur specifically when creating or destroying the VkInstance.
      //
      auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
      if (func == nullptr) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to set up debug logging: failed to look up API: vkCreateDebugUtilsMessengerEXT.");
      }
      auto params = _get_debug_create_params();
      auto result = func(this->instance, &params, nullptr, &this->debugMessenger);
      if (result != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to set up debug logging: API call failed.");
      }
   }
}
void DovahKitVulkanSubsystem::setupRenderWindowSurface() {
   auto& rw = this->surfaces.render_window;
   auto create_info = VkWin32SurfaceCreateInfoKHR{
      .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = GetModuleHandle(nullptr),
      .hwnd      = (HWND)rw.widget->winId(),
   };
   if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &rw.surface) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create Render Window surface.");
   }
}
void DovahKitVulkanSubsystem::setupPhysicalDevice() {
   uint32_t count = 0;
   vkEnumeratePhysicalDevices(this->instance, &count, nullptr);
   if (count == 0) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to find a graphics card with Vulkan support.");
   }
   std::vector<VkPhysicalDevice> devices(count);
   vkEnumeratePhysicalDevices(this->instance, &count, devices.data());
   //
   size_t  highest_index = 0;
   int32_t highest_score = 0;
   for (size_t i = 0; i < count; ++i) {
      auto score = deviceScore(devices[i]);
      if (score > highest_score) {
         highest_score = score;
         highest_index = i;
      }
   }
   if (highest_score > 0) {
      this->devices.physical = devices[highest_index];
   }
   if (this->devices.physical == VK_NULL_HANDLE) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to find a graphics card with Vulkan support.");
   }
}
void DovahKitVulkanSubsystem::setupLogicalDevice() {
   assert(this->devices.physical != VK_NULL_HANDLE);
   //
   auto indices = QueueFamilies(this->devices.physical);
   //
   float queue_priority = 1.0F;
   std::vector<VkDeviceQueueCreateInfo> queue_infos;
   {
      queue_infos.reserve(QueueFamilies::unique_family_count);
      //
      for (size_t i = 0; i < QueueFamilies::unique_family_count; ++i) {
         if (!indices.has_index(i))
            continue;
         bool already_used = false;
         for (size_t j = 0; j < i; ++j) {
            if (indices.families.list[j] == indices.families.list[i]) {
               already_used = true;
               break;
            }
         }
         if (already_used)
            //
            // It's possible for queue families to share an index, but we need to make 
            // sure that we create only one queue-info for each index.
            //
            continue;
         //
         queue_infos.push_back(VkDeviceQueueCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.families.list[i],
            .queueCount       = 1,
            .pQueuePriorities = &queue_priority,
         });
      }
   }
   //
   VkPhysicalDeviceFeatures deviceFeatures{};
   auto create_info = VkDeviceCreateInfo{
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount    = (uint32_t)queue_infos.size(),
      .pQueueCreateInfos       = queue_infos.data(),
      .enabledExtensionCount   = (uint32_t)device_extensions.size(),
      .ppEnabledExtensionNames = device_extensions.data(),
      .pEnabledFeatures        = &deviceFeatures,
   };
   if (enable_validation_layers) {
      create_info.enabledLayerCount   = static_cast<uint32_t>(desired_validation_layers.size());
      create_info.ppEnabledLayerNames = desired_validation_layers.data();
   } else {
      create_info.enabledLayerCount = 0;
   }
   if (vkCreateDevice(this->devices.physical, &create_info, nullptr, &this->devices.logical) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create logical device.");
   }
   //
   vkGetDeviceQueue(this->devices.logical, indices.families.graphics,     0, &this->queues.graphics);
   vkGetDeviceQueue(this->devices.logical, indices.families.presentation, 0, &this->queues.presentation);
}
void DovahKitVulkanSubsystem::setupSwapChain() {
   auto deets = swap_chain_support_info(this->devices.physical, this->surfaces.render_window.surface);
   auto& sc = this->swap_chain;
   //
   VkSurfaceFormatKHR surfaceFormat;
   VkPresentModeKHR   presentMode;
   VkExtent2D         extent;
   uint32_t           imageCount;
   //
   #pragma region choose format
      assert(!deets.formats.empty());
      surfaceFormat = deets.formats[0]; // fallback
      for (const auto& current_format : deets.formats) {
         if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB && current_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = current_format;
            break;
         }
      }
      sc.format = surfaceFormat.format;
   #pragma endregion
   #pragma region choose presentation mode
      presentMode = VK_PRESENT_MODE_FIFO_KHR; // fallback
      for (const auto& current_mode : deets.presentation_modes) {
         if (current_mode == desired_swap_chain_presentation_mode) {
            presentMode = current_mode;
            break;
         }
      }
   #pragma endregion
   #pragma region choose extent
      if (deets.capabilities.currentExtent.width != UINT32_MAX) {
         extent = deets.capabilities.currentExtent;
      } else {
         auto& min_e = deets.capabilities.minImageExtent;
         auto& max_e = deets.capabilities.maxImageExtent;
         //
         QWidget* widget = this->surfaces.render_window.widget;
         assert(widget);
         VkExtent2D actualExtent = {
            (uint32_t)widget->width(),
            (uint32_t)widget->height()
         };
         actualExtent.width  = std::clamp(actualExtent.width,  min_e.width,  max_e.width);
         actualExtent.height = std::clamp(actualExtent.height, min_e.height, max_e.height);
         extent = actualExtent;
      }
      sc.extent = extent;
   #pragma endregion
   #pragma region choose image count
      imageCount = deets.capabilities.minImageCount + 1;
      if (deets.capabilities.maxImageCount > 0 && imageCount > deets.capabilities.maxImageCount) {
         imageCount = deets.capabilities.maxImageCount;
      }
   #pragma endregion
   //
   auto create_info = VkSwapchainCreateInfoKHR{
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = this->surfaces.render_window.surface,
      .minImageCount    = imageCount,
      .imageFormat      = surfaceFormat.format,
      .imageColorSpace  = surfaceFormat.colorSpace,
      .imageExtent      = extent,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
   };
   //
   auto indices = QueueFamilies(this->devices.physical);
   auto list    = indices.families.list;
   if (indices.families.graphics != indices.families.presentation) {
      //
      // TODO: Apparently "exclusive" is faster for this case, but requires more complicated setup, 
      //       which the tutorial I'm following feels should be saved for later.
      // 
      // See: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain#page_Creating-the-swap-chain
      //
      create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = list.size();
      create_info.pQueueFamilyIndices   = list.data();
   } else {
      create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
      create_info.queueFamilyIndexCount = 0;       // clearing these two values is optional, but feels cleaner to me
      create_info.pQueueFamilyIndices   = nullptr; //
   }
   create_info.preTransform   = deets.capabilities.currentTransform; // don't rotate or otherwise transform the image while rendering
   create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // disable alpha
   create_info.presentMode    = presentMode;
   create_info.clipped        = VK_TRUE;        // disable rendering of pixels covered (e.g. by other windows); good optimization, but prevents querying the colors of those pixels (e.g. for saving snapshots)
   create_info.oldSwapchain   = VK_NULL_HANDLE; // must be specified when rebuilding a swap chain; keep null for making a new swap chain
   //
   if (vkCreateSwapchainKHR(this->devices.logical, &create_info, nullptr, &sc.handle) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create swap chain.");
   }
   //
   // Get swap chain images, to render to later:
   //
   vkGetSwapchainImagesKHR(this->devices.logical, sc.handle, &imageCount, nullptr);
   sc.images.resize(imageCount);
   vkGetSwapchainImagesKHR(this->devices.logical, sc.handle, &imageCount, sc.images.data());
}
void DovahKitVulkanSubsystem::setupImageViews() {
   auto& sc = this->swap_chain;
   sc.views.resize(sc.images.size());
   for (size_t i = 0; i < sc.images.size(); i++) {
      auto create_info = VkImageViewCreateInfo{
         .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image      = sc.images[i],
         .viewType   = VK_IMAGE_VIEW_TYPE_2D,
         .components = {
            //
            // No color channel mixing/swapping/etc.
            //
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY,
         },
         .subresourceRange = { // control what part of the image is accessed
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0, // don't skip mipmaps
            .levelCount     = 1, // don't use mipmaps
            .baseArrayLayer = 0, // don't skip layers (layers would be useful for stereoscopic 3D, etc.)
            .layerCount     = 1, // only one layer
         },
      };
      if (vkCreateImageView(this->devices.logical, &create_info, nullptr, &sc.views[i]) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create an image view.");
      }
   }
}
void DovahKitVulkanSubsystem::setupRenderPass() {
   auto color_attachment = VkAttachmentDescription{
      .format         = this->swap_chain.format,
      .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
   };
   auto color_attachment_ref = VkAttachmentReference{
      .attachment = 0,
      .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
   };
   //
   auto subpass = VkSubpassDescription{
      .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &color_attachment_ref, // this is actually an array, and indices in it are used directly within shader code
   };
   //
   auto render_pass_info = VkRenderPassCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments    = &color_attachment,
      .subpassCount    = 1,
      .pSubpasses      = &subpass,
   };
   if (vkCreateRenderPass(this->devices.logical, &render_pass_info, nullptr, &this->render_pass) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create render pass.");
   }
}
void DovahKitVulkanSubsystem::setupGraphicsPipeline() {
   QByteArray frag = QResource("shaders/shader.frag.spv").uncompressedData();
   QByteArray vert = QResource("shaders/shader.vert.spv").uncompressedData();
   if (frag.isNull()) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to load fragment shader.");
   }
   if (vert.isNull()) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to load vertex shader.");
   }
   VkShaderModule frag_module = createShaderModule(frag);
   VkShaderModule vert_module = createShaderModule(vert);
   //
   auto frag_info = VkPipelineShaderStageCreateInfo{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_module,
      .pName  = "main",
      .pSpecializationInfo = nullptr, // can pass parameters to the shader
   };
   auto vert_info = VkPipelineShaderStageCreateInfo{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_module,
      .pName  = "main",
      .pSpecializationInfo = nullptr, // can pass parameters to the shader
   };
   //
   auto shaderStages = std::array{ frag_info, vert_info };
   //
   auto visc = VkPipelineVertexInputStateCreateInfo{ // describes the format of vertex info to be passed to the vertex shader
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount   = 0,
      .pVertexBindingDescriptions      = nullptr,
      .vertexAttributeDescriptionCount = 0,
      .pVertexAttributeDescriptions    = nullptr,
   };
   auto iasc = VkPipelineInputAssemblyStateCreateInfo{ // describes how to generate triangles from the vertices we're passing in
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE,
   };
   //
   auto viewport = VkViewport{ // describe what part of the framebuffer we should draw to
      .x        = 0.0,
      .y        = 0.0,
      .width    = (float)this->swap_chain.extent.width,
      .height   = (float)this->swap_chain.extent.height,
      .minDepth = 0.0, // must be >= 0
      .maxDepth = 1.0, // must be <= 1
   };
   auto scissor = VkRect2D{ // describe what part of the framebuffer we should retain (like a write-mask)
      .offset = {0, 0},
      .extent = this->swap_chain.extent,
   };
   auto viewport_create = VkPipelineViewportStateCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .pViewports    = &viewport,
      .scissorCount  = 1,
      .pScissors     = &scissor,
   };
   //
   auto rasterizer_create = VkPipelineRasterizationStateCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE, // setting this to true basically disables the rasterizer entirely
      .polygonMode      = VK_POLYGON_MODE_FILL,
      .cullMode         = VK_CULL_MODE_BACK_BIT,   // cull backfaces, frontfaces (why? lol), or no faces
      .frontFace        = VK_FRONT_FACE_CLOCKWISE, // specify which vertex order (clockwise or counterclockwise) signifies a face pointing toward us
      .depthBiasEnable         = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp          = 0.0f,
      .depthBiasSlopeFactor    = 0.0f,
      .lineWidth = 1.0,
   };
   auto multisample_info = VkPipelineMultisampleStateCreateInfo{ // MSAA (multisampling anti-alias); just disable it
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = VK_FALSE,
      .minSampleShading      = 1.0F,
      .pSampleMask           = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable      = VK_FALSE,
   };
   auto color_blend_attach_info = VkPipelineColorBlendAttachmentState{
      //
      // When drawing onto the framebuffer, how should we paint overtop the previous image?
      //
      .blendEnable         = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,  // These are basically like "alpha" values for traditional blending, but they're only used 
      .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // to influence the RGB color components.
      .colorBlendOp        = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,  // These are basically like "alpha" values for traditional blending, but they're only used 
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // to influence the A color component.
      .alphaBlendOp        = VK_BLEND_OP_ADD,
      .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
   };
   auto color_blend_create_info = VkPipelineColorBlendStateCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_FALSE,         // Enables bitwise-operation blending. Mutually exclusive with "attachment state" 
      .logicOp         = VK_LOGIC_OP_COPY, // blending and will disable that.
      .attachmentCount = 1,
      .pAttachments    = &color_blend_attach_info,
      .blendConstants  = { 0.0f, 0.0f, 0.0f, 0.0f },
   };
   //
   // Now let's create the pipeline layout.
   //
   auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount         = 0,
      .pSetLayouts            = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges    = nullptr,
   };
   if (vkCreatePipelineLayout(this->devices.logical, &pipeline_layout_info, nullptr, &this->pipeline_layout) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create pipeline layout.");
   }
   //
   // Next, the pipeline itself.
   //
   auto pipeline_info = VkGraphicsPipelineCreateInfo{
      .sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = (uint32_t)shaderStages.size(),
      .pStages    = shaderStages.data(), 
      //
      .pVertexInputState   = &visc,
      .pInputAssemblyState = &iasc,
      .pViewportState      = &viewport_create,
      .pRasterizationState = &rasterizer_create,
      .pMultisampleState   = &multisample_info,
      .pDepthStencilState  = nullptr,
      .pColorBlendState    = &color_blend_create_info,
      .pDynamicState       = nullptr,
      //
      .layout = this->pipeline_layout,
      //
      .renderPass = this->render_pass,
      .subpass    = 0,
      //
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1,
   };
   if (vkCreateGraphicsPipelines(this->devices.logical, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &this->pipeline) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create graphics pipeline.");
   }
   //
   // We're done, so we can ditch these shader objects now:
   //
   vkDestroyShaderModule(this->devices.logical, frag_module, nullptr);
   vkDestroyShaderModule(this->devices.logical, vert_module, nullptr);
}

/*static*/ VkDebugUtilsMessengerCreateInfoEXT DovahKitVulkanSubsystem::_get_debug_create_params() {
   if constexpr (!enable_debug_logging) {
      return { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
   }
   return {
      .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = &debugCallback,
      .pUserData       = nullptr,
   };
}
/*static*/ VKAPI_ATTR VkBool32 VKAPI_CALL DovahKitVulkanSubsystem::debugCallback(
   VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
   VkDebugUtilsMessageTypeFlagsEXT messageType,
   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
   void* pUserData
) {
   if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      qDebug("[Vulkan][Validation Layer] %s", pCallbackData->pMessage);
   }
   return VK_FALSE;
}
