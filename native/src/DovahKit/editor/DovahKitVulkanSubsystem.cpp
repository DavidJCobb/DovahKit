#include "DovahKitVulkanSubsystem.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>
#include <QFile>
#include <QResource>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// testing:
#include <QBuffer>
#include <QImage>
#include <QImageReader>

namespace {
   static constexpr auto desired_swap_chain_presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;

   static constexpr size_t frame_in_flight_count = 2;

   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;
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

DovahKitVulkanWidget::DovahKitVulkanWidget(QWidget* parent) : QWidget(parent) {
   this->setAttribute(Qt::WA_OpaquePaintEvent, true);
   this->setAttribute(Qt::WA_PaintOnScreen,    true);
   this->winId(); // force the widget to have a unique HWND
}
void DovahKitVulkanWidget::hideEvent(QHideEvent* event) {
   this->killTimer(this->timerID);
   this->timerID = 0;
   //
   auto& vulkan = DovahKitVulkanSubsystem::get();
   if (!vulkan.isInitialized())
      return;
   vulkan.renderWindowStateChange(this->size(), this->isVisible());
}
void DovahKitVulkanWidget::paintEvent(QPaintEvent* event) {
   auto& vulkan = DovahKitVulkanSubsystem::get();
   if (!vulkan.isInitialized())
      return;
   vulkan.drawFrame();
}
void DovahKitVulkanWidget::resizeEvent(QResizeEvent* event) {
   auto& vulkan = DovahKitVulkanSubsystem::get();
   if (!vulkan.isInitialized())
      return;
   vulkan.renderWindowStateChange(this->size(), this->isVisible());
}
void DovahKitVulkanWidget::showEvent(QShowEvent* event) {
   this->timerID = this->startTimer(16, Qt::PreciseTimer);
   //
   auto& vulkan = DovahKitVulkanSubsystem::get();
   if (!vulkan.isInitialized())
      return;
   vulkan.renderWindowStateChange(this->size(), this->isVisible());
}
void DovahKitVulkanWidget::timerEvent(QTimerEvent* event) {
   this->repaint();
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

#pragma region vertex
/*static*/ std::array<VkVertexInputAttributeDescription, 3> DovahKitVulkanSubsystem::vertex::getAttributeDescriptions() {
   return {
      VkVertexInputAttributeDescription{
         .location = 0, // should match the location value in the shader's code
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT, // vec2
         .offset   = offsetof(vertex, pos),
      },
      VkVertexInputAttributeDescription{ // vertex color
         .location = 1,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT,
         .offset   = offsetof(vertex, color),
      },
      VkVertexInputAttributeDescription{ // UVs
         .location = 2,
         .binding  = 0,
         .format   = VK_FORMAT_R32G32_SFLOAT,
         .offset   = offsetof(vertex, texCoord),
      },
   };
}
/*static*/ VkVertexInputBindingDescription DovahKitVulkanSubsystem::vertex::getBindingDescription() {
   return VkVertexInputBindingDescription{
      .binding   = 0,
      .stride    = sizeof(vertex),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // used for non-instanced rendering
   };
}
#pragma endregion

DovahKitVulkanSubsystem::DovahKitVulkanSubsystem() {
   auto& rw = this->surfaces.render_window;
   rw.widget = new DovahKitVulkanWidget;
   //
   // Do not call (initialize) here. There are some cases where we need to re-access the 
   // singleton via its getter, but that breaks if the singleton is still being constructed.
   //
   this->frames_in_flight.resize(frame_in_flight_count);
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
   this->setupDescriptorSetLayout();
   this->setupGraphicsPipeline();
   this->setupFramebuffers();
   this->setupCommandPool();
   this->setupTestTexture();
   this->setupTestTextureView();
   this->setupTextureSampler();
   this->setupVertexBuffer();
   this->setupIndexBuffer();
   this->setupUniformBuffers();
   this->setupDescriptorPool();
   this->setupDescriptorSets();
   this->setupCommandBuffers();
   this->setupSemaphores();
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
   auto device = this->devices.logical;
   vkDeviceWaitIdle(device); // wait for all draw commands to finish (remember: they're asynch)
   //
   // TODO: Ensure all child objects belonging to the instance are destroyed first.
   //
   this->teardownSwapChain();
   vkDestroySampler(device, this->texture_sampler, nullptr);
   vkDestroyImageView(device, this->test_texture.view,   nullptr);
   vkDestroyImage    (device, this->test_texture.image,  nullptr);
   vkFreeMemory      (device, this->test_texture.memory, nullptr);
   vkDestroyDescriptorSetLayout(device, this->descriptor_set_layout, nullptr); // don't teardown with the swap chain; we may reuse it
   vkDestroyBuffer(device, this->index_buffer, nullptr);
   vkFreeMemory   (device, this->index_buffer_memory, nullptr);
   vkDestroyBuffer(device, this->vertex_buffer, nullptr);
   vkFreeMemory   (device, this->vertex_buffer_memory, nullptr);
   for (auto& frame : this->frames_in_flight) {
      vkDestroySemaphore(device, frame.semaphores.render_finished, nullptr);
      vkDestroySemaphore(device, frame.semaphores.image_available, nullptr);
      vkDestroyFence(device, frame.fence, nullptr);
   }
   vkDestroyCommandPool(device, this->command_pool, nullptr);
   vkDestroyDevice(device, nullptr);
   //
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

VkCommandBuffer DovahKitVulkanSubsystem::beginSingleTimeCommands() {
   //
   // TODO: This is a useful helper function, but you'll actually get higher throughput if you 
   // reuse a single command buffer instead of spawning several temporary buffers; you'd want 
   // to have a function to create that single reusable buffer, and a "flush" function to 
   // execute whatever commands have been recorded so far.
   // 
   // See the end of: https://vulkan-tutorial.com/en/Texture_mapping/Images#page_Transition-barrier-masks
   //
   auto alloc_info = VkCommandBufferAllocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = this->command_pool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
   };
   VkCommandBuffer single_time_command_buffer;
   vkAllocateCommandBuffers(this->devices.logical, &alloc_info, &single_time_command_buffer);
   //
   auto begin_info = VkCommandBufferBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
   };
   vkBeginCommandBuffer(single_time_command_buffer, &begin_info);
   //
   return single_time_command_buffer;
}
void DovahKitVulkanSubsystem::endSingleTimeCommands(VkCommandBuffer single_time_command_buffer) {
   vkEndCommandBuffer(single_time_command_buffer);
   //
   auto submit_info = VkSubmitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &single_time_command_buffer,
   };
   vkQueueSubmit(this->queues.graphics, 1, &submit_info, VK_NULL_HANDLE);
   vkQueueWaitIdle(this->queues.graphics);
   //
   vkFreeCommandBuffers(this->devices.logical, this->command_pool, 1, &single_time_command_buffer);
}

void DovahKitVulkanSubsystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
   auto commandBuffer = this->beginSingleTimeCommands();

   auto copy_region = VkBufferCopy{
      .size = size,
   };
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copy_region);

   this->endSingleTimeCommands(commandBuffer);
}
void DovahKitVulkanSubsystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const {
   auto buffer_info = VkBufferCreateInfo{
      .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size         = size,
      .usage        = usage,
      .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
   };
   if (vkCreateBuffer(this->devices.logical, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create vertex buffer.");
   }
   //
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(this->devices.logical, buffer, &memRequirements);
   //
   // In a real-world application, you wouldn't use vkAllocateMemory for each individual object you wish 
   // to render, because there's actually a limit on the number of allocations you can make irrespective 
   // of their total size. Even on high-end hardware, that limit may be in the low thousands, the Vulkan 
   // tutorial gives 4096 as a plausible limit for  hardware like an NVIDIA GTX 1080. What you'd want to 
   // do instead, then, is allocate memory in larger blocks and then manually divide those blocks up for 
   // different objects -- similar to what you'd do when making a block allocator.
   //
   auto alloc_info = VkMemoryAllocateInfo{
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = memRequirements.size,
      .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties),
   };
   if (vkAllocateMemory(this->devices.logical, &alloc_info, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate vertex buffer memory!");
   }

   vkBindBufferMemory(this->devices.logical, buffer, bufferMemory, 0);
}
VkImageView DovahKitVulkanSubsystem::createImageView(VkImage image, VkFormat format) const {
   auto view_info = VkImageViewCreateInfo{
      .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image      = image,
      .viewType   = VK_IMAGE_VIEW_TYPE_2D,
      .format     = format,
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
   VkImageView imageView;
   if (vkCreateImageView(this->devices.logical, &view_info, nullptr, &imageView) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem][createImageView] Failed to create texture image view.");
   }
   return imageView;

}
void DovahKitVulkanSubsystem::createVkImage(uint32_t w, uint32_t h, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& out_image, VkDeviceMemory& out_memory) const {
   auto image_info = VkImageCreateInfo{
      .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .flags     = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format    = format, // TODO: if I write a function to convert between Qt and Vulkan format enums, we can use potentially any format, though not all cards support all formats
      .extent    = {
         .width  = w,
         .height = h,
         .depth  = 1,
      },
      .mipLevels     = 1,
      .arrayLayers   = 1,
      .samples       = VK_SAMPLE_COUNT_1_BIT,
      .tiling        = VK_IMAGE_TILING_OPTIMAL,
      .usage         = usage,
      .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
   };
   if (vkCreateImage(this->devices.logical, &image_info, nullptr, &out_image) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image!");
   }
   //
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(this->devices.logical, out_image, &memRequirements);
   //
   auto alloc_info = VkMemoryAllocateInfo{
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = memRequirements.size,
      .memoryTypeIndex = this->findMemoryType(memRequirements.memoryTypeBits, properties),
   };
   if (vkAllocateMemory(this->devices.logical, &alloc_info, nullptr, &out_memory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
   }
   //
   vkBindImageMemory(this->devices.logical, out_image, out_memory, 0);
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
   if (!features.samplerAnisotropy) { // require anisotropic filtering support
      //
      // TODO: We don't actually need to REQUIRE anisotropic filtering, if we instead just 
      //       remember whether the physical device we chose to use has support for it. We 
      //       can just disable it (when setting up our logical device and when creating 
      //       our texture sampler) if it's unsupported.
      //
      return 0;
   }
   if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { // dedicated graphics card (i.e. not integrated graphics)
      score += 10000;
   }
   score += (std::min)((uint32_t)8192, properties.limits.maxImageDimension2D); // max texture size
   //
   return score;
}
uint32_t DovahKitVulkanSubsystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties(this->devices.physical, &memProperties);
   //
   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) == 0)
         continue;
      if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
         return i;
      }
   }
   throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to find suitable memory type.");
}

void DovahKitVulkanSubsystem::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
   VkCommandBuffer commandBuffer = beginSingleTimeCommands();

   auto region = VkBufferImageCopy{
      .bufferOffset      = 0,
      .bufferRowLength   = 0, // amount of padding bytes between rows?
      .bufferImageHeight = 0, // amount of padding bytes... somewhere?
      .imageSubresource  = {
         .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
         .mipLevel       = 0,
         .baseArrayLayer = 0,
         .layerCount     = 1,
      },
      .imageOffset = { 0,     0,      0 },
      .imageExtent = { width, height, 1 },
   };
   vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
   );

   endSingleTimeCommands(commandBuffer);
}
void DovahKitVulkanSubsystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
   VkCommandBuffer commandBuffer = beginSingleTimeCommands();

   auto barrier = VkImageMemoryBarrier{
      .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask       = 0, // TODO
      .dstAccessMask       = 0, // TODO
      .oldLayout           = oldLayout, // can use VK_IMAGE_LAYOUT_UNDEFINED if you don't care to preserve the image's existing content
      .newLayout           = newLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image               = image,
      .subresourceRange    = {
         .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
         .baseMipLevel   = 0,
         .levelCount     = 1,
         .baseArrayLayer = 0,
         .layerCount     = 1,
      },
   };

   //
   // We need to set up the proper access masks and indicate when (i.e. during what pipeline 
   // stages) we can read and write. We need to handle different transitions here, so we'll 
   // need to extend this function as we add more.
   //
   VkPipelineStageFlags sourceStage;
   VkPipelineStageFlags destinationStage;
   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      //
      // If we're going from an undefined layout (i.e. we don't care about the image's prior 
      // content) to a transfer-destination layout, then our transfer writes don't need to 
      // wait on anything.
      // 
      // Because transfer writes don't need to wait, we can specify an empty access mask and 
      // use the earliest possible pipeline stage: "top of pipe."
      //
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      //
      sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
   } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      //
      // If we're going from a transfer-destination layout to a shader-read-only layout (i.e. 
      // the fragment shader wants to read the image), then we need to wait on transfer writes.
      //
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // wait on transfer writes
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;    // we're doing a shader read
      //
      sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // do this when processing the fragment shader
   } else {
      throw std::invalid_argument("[DovahKitVulkanSubsystem][transitionImageLayout] Unsupported layout transition!");
   }

   vkCmdPipelineBarrier(
      commandBuffer,
      sourceStage, destinationStage,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
   );

   endSingleTimeCommands(commandBuffer);
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
   auto deviceFeatures = VkPhysicalDeviceFeatures{
      .samplerAnisotropy = VK_TRUE, 
   };
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
      sc.views[i] = this->createImageView(sc.images[i], this->swap_chain.format);
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
   auto dependency = VkSubpassDependency{
      .srcSubpass    = VK_SUBPASS_EXTERNAL,
      .dstSubpass    = 0,
      .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
   };
   //
   auto render_pass_info = VkRenderPassCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments    = &color_attachment,
      .subpassCount    = 1,
      .pSubpasses      = &subpass,
      .dependencyCount = 1,
      .pDependencies   = &dependency,
   };
   if (vkCreateRenderPass(this->devices.logical, &render_pass_info, nullptr, &this->render_pass) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create render pass.");
   }
}
void DovahKitVulkanSubsystem::setupDescriptorSetLayout() {
   std::array bindings = {
      VkDescriptorSetLayoutBinding{ // uniform buffer object
         .binding            = 0,
         .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount    = 1,
         .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
         .pImmutableSamplers = nullptr,
      },
      VkDescriptorSetLayoutBinding{ // texture sampler
         .binding            = 1,
         .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount    = 1,
         .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
         .pImmutableSamplers = nullptr,
      },
   };
   //
   auto layout_info = VkDescriptorSetLayoutCreateInfo{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = bindings.size(),
      .pBindings    = bindings.data(),
   };
   if (vkCreateDescriptorSetLayout(this->devices.logical, &layout_info, nullptr, &this->descriptor_set_layout) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create descriptor set layout.");
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
   auto vert_binding    = vertex::getBindingDescription();
   auto vert_attributes = vertex::getAttributeDescriptions();
   auto visc = VkPipelineVertexInputStateCreateInfo{ // describes the format of vertex info to be passed to the vertex shader
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount   = 1,
      .pVertexBindingDescriptions      = &vert_binding,
      .vertexAttributeDescriptionCount = (uint32_t)vert_attributes.size(),
      .pVertexAttributeDescriptions    = vert_attributes.data(),
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
      .frontFace        = VK_FRONT_FACE_COUNTER_CLOCKWISE, // specify which vertex order (clockwise or counterclockwise) signifies a face pointing toward us
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
      .setLayoutCount         = 1,
      .pSetLayouts            = &this->descriptor_set_layout,
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
void DovahKitVulkanSubsystem::setupFramebuffers() {
   auto& sc = this->swap_chain;
   sc.framebuffers.resize(sc.views.size());
   for (size_t i = 0; i < sc.views.size(); i++) {
      auto attachments = std::array{ sc.views[i] };
      auto framebuffer_info = VkFramebufferCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
         .renderPass      = this->render_pass,
         .attachmentCount = attachments.size(),
         .pAttachments    = attachments.data(),
         .width           = sc.extent.width,
         .height          = sc.extent.height,
         .layers          = 1,
      };
      if (vkCreateFramebuffer(this->devices.logical, &framebuffer_info, nullptr, &sc.framebuffers[i]) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create a framebuffer.");
      }
   }
}
void DovahKitVulkanSubsystem::setupCommandPool() {
   auto indices   = QueueFamilies(this->devices.physical);
   auto pool_info = VkCommandPoolCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = 0,
      .queueFamilyIndex = indices.families.graphics,
   };
   if (vkCreateCommandPool(this->devices.logical, &pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the command pool.");
   }
}
void DovahKitVulkanSubsystem::setupTestTexture() {
   QImage texture;
   {
      auto bytearray = QResource("shaders/Tamriel-Skyrim.esm.png").uncompressedData();
      auto buffer    = QBuffer(&bytearray);
      buffer.open(QIODevice::ReadOnly);
      auto reader    = QImageReader(&buffer, "PNG");
      reader.read(&texture);
      texture = texture.convertToFormat(QImage::Format::Format_RGBA8888);
   }
   if (texture.isNull()) {
      throw std::runtime_error("[DovahKitVulkanSubsystem][setupTestTexture] Failed to load test image.");
   }
   VkDeviceSize image_size = texture.width() * texture.height() * 4;
   assert(image_size == texture.sizeInBytes());
   //
   // We're gonna be setting up our image on a staging buffer, and then transferring that 
   // to the final (non-CPU-writeable) buffer.
   //
   VkBuffer       staging_buffer;
   VkDeviceMemory staging_memory;
   this->createBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
   //
   void* data;
   vkMapMemory(this->devices.logical, staging_memory, 0, image_size, 0, &data);
   memcpy(data, texture.constBits(), image_size);
   vkUnmapMemory(this->devices.logical, staging_memory);
   //
   uint32_t w = texture.width();
   uint32_t h = texture.height();
   texture = QImage();
   //
   // Now let's create an image:
   //
   this->createVkImage(
      w, h,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      this->test_texture.image,
      this->test_texture.memory
   );
   //
   // Now we need to transfer our image from the staging buffer to the final buffer, 
   // transitioning its layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL as we do. We 
   // can use VK_IMAGE_LAYOUT_UNDEFINED as the "old layout" because we don't actually 
   // care about the data (or lack thereof, really) in the freshly-created VkImage.
   //
   this->transitionImageLayout(this->test_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   this->copyBufferToImage(staging_buffer, this->test_texture.image, w, h);
   this->transitionImageLayout(this->test_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
   //
   // Discard the staging buffer:
   //
   vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
   vkFreeMemory   (this->devices.logical, staging_memory, nullptr);
}
void DovahKitVulkanSubsystem::setupTestTextureView() {
   this->test_texture.view = createImageView(this->test_texture.image, VK_FORMAT_R8G8B8A8_SRGB);
}
void DovahKitVulkanSubsystem::setupTextureSampler() {
   auto sampler_info = VkSamplerCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter        = VK_FILTER_LINEAR,
      .minFilter        = VK_FILTER_LINEAR,
      .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias       = 0.0,
      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy    = 8,
      .compareEnable    = VK_FALSE,
      .compareOp        = VK_COMPARE_OP_ALWAYS,
      .minLod           = 0.0,
      .maxLod           = 0.0,
      .borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE, // true: coordinates are [0, width], etc; false: coordinates are [0, 1]
   };
   {  // Constrain based on device capabilities.
      VkPhysicalDeviceProperties properties{};
      vkGetPhysicalDeviceProperties(this->devices.physical, &properties);
      sampler_info.maxAnisotropy = std::min(sampler_info.maxAnisotropy, properties.limits.maxSamplerAnisotropy);
   }
   if (vkCreateSampler(this->devices.logical, &sampler_info, nullptr, &this->texture_sampler) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the texture sampler.");
   }
}
void DovahKitVulkanSubsystem::setupVertexBuffer() {
   VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();
   //
   // Our normal vertex buffer is going to have a flag set which renders its contents entirely 
   // inaccessible to the CPU; this aids in performance. But how, then, shall we get our vertices 
   // from the CPU to the GPU? We'll use a staging buffer -- a temporary GPU-side buffer which 
   // lacks this flag.
   //
   VkBuffer       staging_buffer;
   VkDeviceMemory staging_memory;
   this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
   //
   // We want to transfer our vertex data to the GPU. We'll do this by mapping a section of 
   // CPU-accessible memory, copying the data into that section, and then unmapping it.
   //
   void* data;
   vkMapMemory(this->devices.logical, staging_memory, 0, bufferSize, 0, &data);
   memcpy(data, this->vertices.data(), (size_t)bufferSize);
   vkUnmapMemory(this->devices.logical, staging_memory);
   //
   // Now let's create our normal buffer, and transfer data from the staging buffer to the 
   // normal buffer.
   //
   this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertex_buffer, this->vertex_buffer_memory);
   this->copyBuffer(staging_buffer, this->vertex_buffer, bufferSize);
   //
   // And of course, let's destroy the temporary buffer and its memory:
   //
   vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
   vkFreeMemory(this->devices.logical, staging_memory, nullptr);
}
void DovahKitVulkanSubsystem::setupIndexBuffer() {
   VkDeviceSize bufferSize = sizeof(this->indices[0]) * this->indices.size();
   //
   // The index buffer allows us to reuse vertices without having to re-specify them, in the 
   // case where multiple triangles share a vertex or few.
   //
   // A side note: device drivers can optimize better if we actually store the vertex and index 
   // data in the same buffer, and use the "offset" parameter in vkCmdBindVertexBuffers (when 
   // setting up our command buffers) to indicate where one ends and the other begins.
   //
   VkBuffer       staging_buffer;
   VkDeviceMemory staging_memory;
   this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
   //
   void* data;
   vkMapMemory(this->devices.logical, staging_memory, 0, bufferSize, 0, &data);
   memcpy(data, indices.data(), (size_t)bufferSize);
   vkUnmapMemory(this->devices.logical, staging_memory);
   //
   this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->index_buffer, this->index_buffer_memory);
   this->copyBuffer(staging_buffer, this->index_buffer, bufferSize);
   //
   vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
   vkFreeMemory(this->devices.logical, staging_memory, nullptr);
}
void DovahKitVulkanSubsystem::setupUniformBuffers() {
   VkDeviceSize buffer_size = sizeof(uniform_buffer_object);
   //
   auto& sc     = this->swap_chain;
   auto& list_b = sc.uniform_buffers;
   auto& list_m = sc.uniform_buffer_memory;
   list_b.resize(sc.images.size());
   list_m.resize(sc.images.size());
   for (size_t i = 0; i < sc.images.size(); i++) {
      createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, list_b[i], list_m[i]);
   }
}
void DovahKitVulkanSubsystem::setupDescriptorPool() {
   auto& sc = this->swap_chain;
   //
   auto pool_sizes = std::array{
      VkDescriptorPoolSize{ // uniform buffer object
         .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = (uint32_t)sc.images.size(),
      },
      VkDescriptorPoolSize{ // texture sampler
         .type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = (uint32_t)sc.images.size(),
      },
   };
   auto pool_info = VkDescriptorPoolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets       = (uint32_t)sc.images.size(),
      .poolSizeCount = (uint32_t)pool_sizes.size(),
      .pPoolSizes    = pool_sizes.data(),
   };
   //
   if (vkCreateDescriptorPool(this->devices.logical, &pool_info, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the descriptor pool.");
   }
}
void DovahKitVulkanSubsystem::setupDescriptorSets() {
   auto& sc = this->swap_chain;

   std::vector<VkDescriptorSetLayout> layouts(sc.images.size(), this->descriptor_set_layout);
   auto alloc_info = VkDescriptorSetAllocateInfo{
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool     = this->descriptor_pool,
      .descriptorSetCount = (uint32_t)sc.images.size(),
      .pSetLayouts        = layouts.data(),
   };
   //
   this->descriptor_sets.resize(sc.images.size());
   //
   // WARNING: If the descriptor pool has an inadequate size, vkAllocateDescriptorSets 
   // MAY fail with an VK_ERROR_POOL_OUT_OF_MEMORY error code... However, some device 
   // drivers may try to solve the problem internally instead, which means that that 
   // particular class of error will not fail consistently across all hardware. Beware. 
   //
   if (vkAllocateDescriptorSets(this->devices.logical, &alloc_info, this->descriptor_sets.data()) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to allocate descriptor sets.");
   }
   //
   // Descriptor sets are owned by their descriptor pool. No manual cleanup needed.
   //
   // Configure the new descriptor sets:
   //
   for (size_t i = 0; i < sc.images.size(); i++) {
      auto buffer_info = VkDescriptorBufferInfo{
         .buffer = sc.uniform_buffers[i],
         .offset = 0,
         .range  = sizeof(uniform_buffer_object), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
      };
      auto image_info = VkDescriptorImageInfo{
         .sampler     = this->texture_sampler,
         .imageView   = this->test_texture.view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      //
      auto descriptor_writes = std::array{
         VkWriteDescriptorSet{ // uniform buffer object
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = this->descriptor_sets[i],
            .dstBinding       = 0, // this should match the binding value in the shader
            .dstArrayElement  = 0, // index of the first descriptor in the raray to update
            .descriptorCount  = 1, // you can update multiple descriptors at once if they're in an array
            .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &buffer_info,
            .pTexelBufferView = nullptr,
         },
         VkWriteDescriptorSet{ // texture sampler
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = this->descriptor_sets[i],
            .dstBinding      = 1, // this should match the binding value in the shader
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo      = &image_info,
         },
      };
      vkUpdateDescriptorSets(this->devices.logical, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
   }
}
void DovahKitVulkanSubsystem::setupCommandBuffers() {
   //
   // Command buffers allow us to "record" several draw commands (possibly across multiple 
   // threads) and then execute them all at once in the main thread. You could think of it 
   // like a transaction, I guess.
   //
   this->command_buffers.resize(this->swap_chain.framebuffers.size());
   auto alloc_info = VkCommandBufferAllocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = this->command_pool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = (uint32_t)this->command_buffers.size(),
   };
   if (vkAllocateCommandBuffers(this->devices.logical, &alloc_info, this->command_buffers.data()) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to allocate command buffers.");
   }
   //
   // I believe the code from here on out is just for deciding what to render -- recording 
   // a command buffer, pretty much. I maybe wouldn't put this inline in the setup method, 
   // but that's how the Vulkan tutorial wants me to do it, at least for now.
   //
   for (size_t i = 0; i < this->command_buffers.size(); i++) {
      auto& command_buffer = this->command_buffers[i];
      //
      auto buffer_begin_info = VkCommandBufferBeginInfo{
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = 0,
         .pInheritanceInfo = nullptr,
      };
      if (vkBeginCommandBuffer(command_buffer, &buffer_begin_info) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to begin recording command buffer.");
      }
      //
      auto clear_values    = std::array{ VkClearValue{0, 0, 0, 1} }; // color(s) to use with the VK_ATTACHMENT_LOAD_OP_CLEAR option, passed in an earlier setup function
      auto pass_begin_info = VkRenderPassBeginInfo{
         .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass  = this->render_pass,
         .framebuffer = this->swap_chain.framebuffers[i],
         .renderArea  = {
            .offset = { 0, 0 },
            .extent = this->swap_chain.extent,
         },
         .clearValueCount = (uint32_t)clear_values.size(),
         .pClearValues    = clear_values.data(),
      };

      vkCmdBeginRenderPass(command_buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
      {
         using index_type = decltype(this->indices)::value_type;
         constexpr bool indices_are_uint32_t = std::is_same_v<uint32_t, index_type>;
         constexpr bool indices_are_uint16_t = std::is_same_v<uint16_t, index_type>;
         static_assert(indices_are_uint32_t || indices_are_uint16_t);
         //
         vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
         //
         std::array buffers = { this->vertex_buffer };
         std::array<VkDeviceSize, buffers.size()> offsets = { 0 };
         vkCmdBindVertexBuffers(command_buffer, 0, buffers.size(), buffers.data(), offsets.data());
         if constexpr (indices_are_uint32_t) {
            vkCmdBindIndexBuffer(command_buffer, this->index_buffer, 0, VK_INDEX_TYPE_UINT32);
         } else if constexpr (indices_are_uint16_t) {
            vkCmdBindIndexBuffer(command_buffer, this->index_buffer, 0, VK_INDEX_TYPE_UINT16);
         }
         //
         vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 0, 1, &this->descriptor_sets[i], 0, nullptr);
         //
         vkCmdDrawIndexed(command_buffer, (uint32_t)this->indices.size(), 1, 0, 0, 0);
      }
      vkCmdEndRenderPass(command_buffer);

      if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to record a command buffer.");
      }
   }
}
void DovahKitVulkanSubsystem::setupSemaphores() {
   auto semaphore_info = VkSemaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
   };
   auto fence_info = VkFenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      //
      // Our drawFrame code waits until a frame is signalled, but frames start off unsignalled by 
      // default. This means that it'll wait forever, unless we initialize the frame as signalled.
      //
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
   };
   for (auto& frame : this->frames_in_flight) {
      if (vkCreateSemaphore(this->devices.logical, &semaphore_info, nullptr, &frame.semaphores.image_available) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create frame-in-flight semaphore (image-available).");
      }
      if (vkCreateSemaphore(this->devices.logical, &semaphore_info, nullptr, &frame.semaphores.render_finished) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create frame-in-flight semaphore (render-finished).");
      }
      if (vkCreateFence(this->devices.logical, &fence_info, nullptr, &frame.fence) != VK_SUCCESS) {
         throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create frame-in-flight fence.");
      }
   }
   //
   // These don't get created; they're just handles.
   //
   this->swap_chain.images_in_flight.resize(this->swap_chain.images.size());
}

void DovahKitVulkanSubsystem::recreateSwapChain() {
   vkDeviceWaitIdle(this->devices.logical);

   this->teardownSwapChain();

   this->setupSwapChain();
   this->setupImageViews();
   this->setupRenderPass();
   this->setupGraphicsPipeline();
   this->setupFramebuffers();
   this->setupUniformBuffers();
   this->setupDescriptorPool();
   this->setupDescriptorSets();
   this->setupCommandBuffers();
}

void DovahKitVulkanSubsystem::teardownSwapChain() {
   auto  device = this->devices.logical;
   auto& sc     = this->swap_chain;
   for (auto framebuffer : sc.framebuffers) {
      vkDestroyFramebuffer(device, framebuffer, nullptr);
   }
   //
   // Free the command buffers, but don't destroy the whole command pool. This saves us 
   // the trouble of having to rebuild the command pool if we're merely rebuilding the 
   // swap chain rather than doing full teardown.
   //
   vkFreeCommandBuffers(device, this->command_pool, (uint32_t)this->command_buffers.size(), this->command_buffers.data());
   //
   for (size_t i = 0; i < sc.images.size(); i++) {
      vkDestroyBuffer(device, sc.uniform_buffers[i], nullptr);
      vkFreeMemory(device, sc.uniform_buffer_memory[i], nullptr);
   }
   vkDestroyDescriptorPool(device, this->descriptor_pool, nullptr);
   vkDestroyPipeline(device, this->pipeline, nullptr);
   vkDestroyPipelineLayout(device, this->pipeline_layout, nullptr);
   vkDestroyRenderPass(device, this->render_pass, nullptr);
   for (auto view : sc.views) {
      vkDestroyImageView(device, view, nullptr);
   }
   vkDestroySwapchainKHR(device, sc.handle, nullptr);
}

void DovahKitVulkanSubsystem::updateUniformBuffer(uint32_t which) {
   auto& sc = this->swap_chain;
   assert(which < sc.images.size());
   //
   static auto start_time = std::chrono::high_resolution_clock::now();
   //
   auto  now     = std::chrono::high_resolution_clock::now();
   float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
   //
   uniform_buffer_object ubo{};
   ubo.model = glm::rotate(glm::mat4(1.0f), elapsed * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   ubo.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   ubo.proj  = glm::perspective(glm::radians(45.0f), sc.extent.width / (float)sc.extent.height, 0.1f, 10.0f);
   //
   // GLM was designed for OpenGL, which uses an inverted Y axis. We need to flip the 
   // Y-axis here. Do be aware, however, that this is a 3D flip; vertex order will 
   // change handedness (clockwise/counterclockwise), which will affect what Vulkan 
   // considers a "backface" versus a "frontface." You can update the handedness in 
   // the setupGraphicsPipeline function.
   //
   ubo.proj[1][1] *= -1;
   //
   // Send data to the GPU:
   //
   void* data;
   vkMapMemory(this->devices.logical, sc.uniform_buffer_memory[which], 0, sizeof(ubo), 0, &data);
   memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(this->devices.logical, sc.uniform_buffer_memory[which]);
}
void DovahKitVulkanSubsystem::drawFrame() {
   constexpr auto no_timeout = UINT64_MAX;
   //
   //  - Acquire an image from the swap chain
   //  - Execute the command buffer with that image as attachment in the framebuffer
   //  - Return the image to the swap chain for presentation
   // 
   // These tasks are asynchronous, but must run sequentially.
   //
   auto& rw = this->surfaces.render_window;
   if (!rw.visible)
      return;
   auto& frame = this->frames_in_flight[this->current_frame];
   this->current_frame = (this->current_frame + 1) % frame_in_flight_count;
   vkWaitForFences(this->devices.logical, 1, &frame.fence, VK_TRUE, no_timeout);
   //
   uint32_t imageIndex;
   VkResult result = vkAcquireNextImageKHR(this->devices.logical, this->swap_chain.handle, no_timeout, frame.semaphores.image_available, VK_NULL_HANDLE, &imageIndex);
   if constexpr (rebuild_swap_chain_asap_if_suboptimal) {
      if (result == VK_SUBOPTIMAL_KHR) {
         this->recreateSwapChain();
         return;
      }
   }
   if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      this->recreateSwapChain();
      return;
   } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to acquire swap chain image!");
   }
   {
      auto& handle = this->swap_chain.images_in_flight[imageIndex];
      //
      // Check if a previous frame is using this image.
      //
      if (handle != VK_NULL_HANDLE) {
         vkWaitForFences(this->devices.logical, 1, &handle, VK_TRUE, UINT64_MAX);
      }
      //
      // Mark the image as now being in use by this frame.
      //
      handle = frame.fence;
   }
   //
   this->updateUniformBuffer(imageIndex);
   auto wait_semaphores   = std::array{ frame.semaphores.image_available };
   auto signal_semaphores = std::array{ frame.semaphores.render_finished };
   VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
   auto submit_info = VkSubmitInfo{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount   = wait_semaphores.size(),
      .pWaitSemaphores      = wait_semaphores.data(),
      .pWaitDstStageMask    = waitStages,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &this->command_buffers[imageIndex],
      .signalSemaphoreCount = signal_semaphores.size(),
      .pSignalSemaphores    = signal_semaphores.data(),
   };
   vkResetFences(this->devices.logical, 1, &frame.fence);
   if (vkQueueSubmit(this->queues.graphics, 1, &submit_info, frame.fence) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
   }
   //
   auto swap_chain_handles = std::array{ this->swap_chain.handle };
   auto presentation_info  = VkPresentInfoKHR{
      .sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount  = signal_semaphores.size(),
      .pWaitSemaphores     = signal_semaphores.data(),
      .swapchainCount      = swap_chain_handles.size(),
      .pSwapchains         = swap_chain_handles.data(),
      .pImageIndices       = &imageIndex,
      .pResults            = nullptr,
   };
   result = vkQueuePresentKHR(this->queues.presentation, &presentation_info);
   if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || rw.resized) {
      rw.resized = false;
      recreateSwapChain();
   } else if (result != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem][drawFrame] Failed to present swap chain image.");
   }
}
void DovahKitVulkanSubsystem::renderWindowStateChange(QSize size, bool visible) {
   auto& rw = this->surfaces.render_window;
   if (size != rw.last_size) {
      rw.last_size = size;
      rw.resized   = true;
   }
   rw.visible = visible && !size.isEmpty();
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
