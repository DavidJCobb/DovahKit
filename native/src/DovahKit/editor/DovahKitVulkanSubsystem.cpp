#include "DovahKitVulkanSubsystem.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>
#include <QFile>
#include <QResource>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// testing:
#include <QBuffer>
#include <QImage>
#include <QImageReader>

namespace {
   //static constexpr int  target_frames_per_second = 60;
   static constexpr int  target_frames_per_second = -1;
   static constexpr uint frame_delay = (target_frames_per_second >= 0) ? 1000 / target_frames_per_second : 0;
}

namespace {
   static constexpr auto desired_swap_chain_presentation_mode = VK_PRESENT_MODE_MAILBOX_KHR;

   static constexpr size_t frame_in_flight_count = 2;

   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;

   static constexpr size_t   max_rendered_objects   = 1024;
   static constexpr uint32_t max_available_textures = 256;
}

namespace {
   static constexpr bool debug_print_all_extensions = false;

   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
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

namespace {
   struct _model {
      using vertex = DovahKitVulkanSubsystem::vertex;
      const std::vector<vertex>   vertices;
      const std::vector<uint16_t> indices;
      glm::mat4 transform;
   };

   std::array texture_files = {
      "Tamriel-Skyrim.esm.png",
      "ScreenShot278.bmp",
      "ScreenShot389.bmp",
   };

   std::array models = {
      _model{  // Skyrim texture plane
         {  // Vertices
            {{-0.5f, -0.395f, 0.0}, {1.0f, 0.0f, 0.0f}, {1.0, 0.0}},
            {{ 0.5f, -0.395f, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0, 0.0}},
            {{ 0.5f,  0.395f, 0.0}, {0.0f, 0.0f, 1.0f}, {0.0, 1.0}},
            {{-0.5f,  0.395f, 0.0}, {1.0f, 1.0f, 1.0f}, {1.0, 1.0}},
         },
         { 0, 1, 2, 2, 3, 0 },
         glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            { 0.0, 0.0, 0.7 }
         ),
      },
      _model{  // screenshot of Tolfdir
         {  // Vertices
            {{-0.5f, -0.28125, 0.0}, {1.0f, 0.0f, 0.0f}, {1.0, 0.0}},
            {{ 0.5f, -0.28125, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0, 0.0}},
            {{ 0.5f,  0.28125, 0.0}, {0.0f, 0.0f, 1.0f}, {0.0, 1.0}},
            {{-0.5f,  0.28125, 0.0}, {1.0f, 1.0f, 1.0f}, {1.0, 1.0}},
         },
         { 0, 1, 2, 2, 3, 0 },
         glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            { 0.0, 0.0, -0.5 }
         ),
      },
      _model{  // screenshot of books
         {  // Vertices
            {{-0.5f, -0.28125, 0.0}, {1.0f, 0.0f, 0.0f}, {1.0, 0.0}},
            {{ 0.5f, -0.28125, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0, 0.0}},
            {{ 0.5f,  0.28125, 0.0}, {0.0f, 0.0f, 1.0f}, {0.0, 1.0}},
            {{-0.5f,  0.28125, 0.0}, {1.0f, 1.0f, 1.0f}, {1.0, 1.0}},
         },
         { 0, 1, 2, 2, 3, 0 },
         glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            { 0.0, 1.0, 0.0 }
         ),
      },
   };
}

namespace {
   bool hasStencilComponent(VkFormat format) {
      return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
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
   this->timerID = this->startTimer(frame_delay, Qt::PreciseTimer);
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

#pragma region shader_module
DovahKitVulkanSubsystem::shader_module::shader_module(VkDevice device, const QByteArray& compiled) : device(device) {
   auto create_info = VkShaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = (uint32_t)compiled.size(),
      .pCode    = (const uint32_t*)compiled.data(),
   };
   if (vkCreateShaderModule(device, &create_info, nullptr, &this->handle) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem::shader_module::shader_module] Failed to create shader module.");
   }
}
DovahKitVulkanSubsystem::shader_module::~shader_module() {
   if (this->device != VK_NULL_HANDLE && this->handle != VK_NULL_HANDLE)
      vkDestroyShaderModule(this->device, this->handle, nullptr);
   this->handle = VK_NULL_HANDLE;
   this->device = VK_NULL_HANDLE;
}
//
DovahKitVulkanSubsystem::shader_module::shader_module(shader_module&& o) noexcept {
   std::swap(this->handle, o.handle);
   std::swap(this->device, o.device);
}
DovahKitVulkanSubsystem::shader_module& DovahKitVulkanSubsystem::shader_module::operator=(shader_module&& o) noexcept {
   std::swap(this->handle, o.handle);
   std::swap(this->device, o.device);
   return *this;
}
#pragma endregion

#pragma region rendered_object
void DovahKitVulkanSubsystem::rendered_object::_on_shader_parameter_change() {
   if (this->pending_delete)
      return;
   this->frame_dirty_flags = -1;
}
//
void DovahKitVulkanSubsystem::rendered_object::set_transform(const glm::mat4& in) {
   this->shader_params.transform = in;
   this->_on_shader_parameter_change();
}
#pragma endregion

#pragma region vertex
/*static*/ std::array<VkVertexInputAttributeDescription, 3> DovahKitVulkanSubsystem::vertex::getAttributeDescriptions() {
   return {
      VkVertexInputAttributeDescription{
         .location = 0, // should match the location value in the shader's code
         .binding  = 0,
         .format   = VK_FORMAT_R32G32B32_SFLOAT, // vec3
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
   //
   this->descriptor_set_layout.bindings = {
      DovahKit::vulkan::descriptor_binding{ // uniform buffer object
         .index              = 0,
         .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .count              = 1,
         .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
         .immutable_samplers = nullptr,
      },
      DovahKit::vulkan::descriptor_binding{ // texture sampler
         .index              = 1,
         .type               = VK_DESCRIPTOR_TYPE_SAMPLER,
         .count              = 1,
         .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
         .immutable_samplers = nullptr,
         //.is_global          = true,
      },
      DovahKit::vulkan::descriptor_binding{ // storage buffer object: rendered_object::shader_parameters
         .index              = 2,
         .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .count              = 1,
         .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
         .immutable_samplers = nullptr,
      },
      DovahKit::vulkan::descriptor_binding{ // texture array
         .index              = 3,
         .flags              = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
         .type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
         .count              = max_available_textures,
         .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         .immutable_samplers = nullptr,
      },
   };
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
   this->setupShaderModules();
   this->setupCommandPool(); // cannot copy buffers, etc., for texture loading until this is set up
   this->setupSwapChain();
   this->setupImageViews();
   this->setupRenderPass();
      this->setupTextures(); // descriptor set layout must know how many textures we want to have room for (currently we use a dynamic count, rather than, say, just having 1000 or something)
      this->setupTextureSampler(); // descriptor set layout must be able to refer to our immutable sampler
   this->setupDescriptorSetLayout();
   this->setupGraphicsPipeline();
   this->setupDepthBuffer();
   this->setupFramebuffers(); // dependency on the depth buffer
   this->setupRenderedObjects();
   this->setupShaderParameterBuffers();
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
   {
      auto& list = this->assets.textures;
      for (auto& entry : list) {
         if (entry.image == VK_NULL_HANDLE) {
            assert(entry.view == VK_NULL_HANDLE && entry.memory == VK_NULL_HANDLE);
            continue;
         }
         vkDestroyImageView(device, entry.view,   nullptr);
         vkDestroyImage    (device, entry.image,  nullptr);
         vkFreeMemory      (device, entry.memory, nullptr);
      }
      list.clear();
   }
   {  // Delete null-texture (used to clear texture-array entries when null-descriptors aren't allowed by the hardware).
      auto& entry = this->null_texture;
      if (entry.image == VK_NULL_HANDLE) {
         assert(entry.view == VK_NULL_HANDLE && entry.memory == VK_NULL_HANDLE);
      } else {
         vkDestroyImageView(device, entry.view,   nullptr);
         vkDestroyImage    (device, entry.image,  nullptr);
         vkFreeMemory      (device, entry.memory, nullptr);
         entry.view   = VK_NULL_HANDLE;
         entry.image  = VK_NULL_HANDLE;
         entry.memory = VK_NULL_HANDLE;
      }
   }
   this->descriptor_set_layout.teardown(); // don't teardown with the swap chain; we may reuse it
   {
      auto& list = this->shader_modules;
      list.clear();
   }
   {
      auto& list = this->rendered_objects;
      for (auto& ro : list) {
         auto& vib = ro.vertex_and_index_buffer;
         if (vib.buffer == VK_NULL_HANDLE) {
            assert(vib.memory == VK_NULL_HANDLE);
         } else {
            vkDestroyBuffer(device, vib.buffer, nullptr);
            vkFreeMemory   (device, vib.memory, nullptr);
            vib.buffer = VK_NULL_HANDLE;
            vib.memory = VK_NULL_HANDLE;
         }
      }
      list.clear();
   }
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
VkImageView DovahKitVulkanSubsystem::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect) const {
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
         .aspectMask     = aspect,
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
   auto indexing_features = VkPhysicalDeviceDescriptorIndexingFeaturesEXT{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
      .pNext = nullptr,
   };
   auto properties = VkPhysicalDeviceProperties{};
   auto features   = VkPhysicalDeviceFeatures2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext = &indexing_features,
   };
   vkGetPhysicalDeviceProperties(device, &properties);
   vkGetPhysicalDeviceFeatures2 (device, &features);
   //
   if (!indexing_features.descriptorBindingVariableDescriptorCount) // variable-length arrays as descriptor bindings
      return 0;
   if (!indexing_features.descriptorBindingPartiallyBound) // arrays with empty slots as descriptor bindings
      return 0;
   if (!indexing_features.runtimeDescriptorArray)
      return 0;
   //
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
VkFormat DovahKitVulkanSubsystem::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
   for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(this->devices.physical, format, &props);
      //
      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
         return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
         return format;
      }
   }
   return VK_FORMAT_UNDEFINED;
}

VkFormat DovahKitVulkanSubsystem::findDepthFormat() const {
   auto fmt = this->findSupportedFormat(
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
   );
   if (fmt == VK_FORMAT_UNDEFINED) {
      throw std::runtime_error("[DovahKitVulkanSubsystem][findDepthFormat] No format.");
   }
   return fmt;
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
   // Handle special-case aspect masks:
   //
   if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      if (hasStencilComponent(format))
         barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
   }
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
   } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      //
      // Transition used when creating a new depth image for our depth buffer.
      //
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      //
      sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
      //
      // Identify support:
      //
      auto robustness = VkPhysicalDeviceRobustness2FeaturesEXT{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
         .pNext = nullptr,
      };
      auto features = VkPhysicalDeviceFeatures2{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
         .pNext = &robustness,
      };
      VkPhysicalDeviceProperties properties{};
      vkGetPhysicalDeviceProperties(this->devices.physical, &properties);
      vkGetPhysicalDeviceFeatures2 (this->devices.physical, &features);
      //
      this->support.anisotropic_filtering = 0;
      this->support.null_descriptors      = robustness.nullDescriptor;
      //
      if (features.features.samplerAnisotropy) {
         this->support.anisotropic_filtering = properties.limits.maxSamplerAnisotropy;
      }
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
   // Note that if we want device features or extensions, we generally have to request them 
   // explicitly.
   //
   auto deviceFeatures = VkPhysicalDeviceFeatures{
      .samplerAnisotropy = this->support.anisotropic_filtering > 0 ? VK_TRUE : VK_FALSE,
   };
   //
   auto robustness_extensions = VkPhysicalDeviceRobustness2FeaturesEXT{
      .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
      .pNext               = nullptr,
      .robustBufferAccess2 = VK_FALSE,
      .robustImageAccess2  = VK_FALSE,
      .nullDescriptor      = this->support.null_descriptors ? VK_TRUE : VK_FALSE,
   };
   auto indexing_extensions = VkPhysicalDeviceDescriptorIndexingFeaturesEXT{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
      .pNext = &robustness_extensions,
      .descriptorBindingPartiallyBound          = VK_TRUE,
      .descriptorBindingVariableDescriptorCount = VK_TRUE,
      .runtimeDescriptorArray                   = VK_TRUE,
   };
   auto create_info = VkDeviceCreateInfo{
      .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext                   = &indexing_extensions,
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
   //
   if (vkCreateDevice(this->devices.physical, &create_info, nullptr, &this->devices.logical) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create logical device.");
   }
   //
   // And lastly, let's get our queues:
   //
   vkGetDeviceQueue(this->devices.logical, indices.families.graphics,     0, &this->queues.graphics);
   vkGetDeviceQueue(this->devices.logical, indices.families.presentation, 0, &this->queues.presentation);
}
void DovahKitVulkanSubsystem::setupShaderModules() {
   QByteArray frag = QResource("shaders/shader.frag.spv").uncompressedData();
   QByteArray vert = QResource("shaders/shader.vert.spv").uncompressedData();
   if (frag.isNull()) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to load fragment shader.");
   }
   if (vert.isNull()) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to load vertex shader.");
   }
   //
   this->shader_modules.emplace_back(this->devices.logical, frag);
   this->shader_modules.emplace_back(this->devices.logical, vert);
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
      sc.views[i] = this->createImageView(sc.images[i], this->swap_chain.format, VK_IMAGE_ASPECT_COLOR_BIT);
   }
}
void DovahKitVulkanSubsystem::setupRenderPass() {
   std::array attachment_descs = {
      VkAttachmentDescription{ // color
         .format         = this->swap_chain.format,
         .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
         .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
         .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
         .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
         .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      },
      VkAttachmentDescription{ // depth
         .format         = this->findDepthFormat(),
         .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
         .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
         .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after drawing, so let the driver decide how best to discard it
         .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
         .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
         .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
   };
   std::array attachment_refs = {
      VkAttachmentReference{ // color
         .attachment = 0,
         .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      },
      VkAttachmentReference{ // depth
         .attachment = 1,
         .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      },
   };
   //
   auto subpass = VkSubpassDescription{
      .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount    = 1,
      .pColorAttachments       = &attachment_refs[0],
      .pDepthStencilAttachment = &attachment_refs[1], // subpasses can only use a single depth-and-stencil attachment
   };
   auto dependency = VkSubpassDependency{
      .srcSubpass    = VK_SUBPASS_EXTERNAL,
      .dstSubpass    = 0,
      .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
   };
   //
   auto render_pass_info = VkRenderPassCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = attachment_descs.size(),
      .pAttachments    = attachment_descs.data(),
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
   this->descriptor_set_layout.set_device(this->devices.logical);
   this->descriptor_set_layout.setup();
}
void DovahKitVulkanSubsystem::setupGraphicsPipeline() {
   auto frag_info = VkPipelineShaderStageCreateInfo{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = this->shader_modules[0].handle,
      .pName  = "main",
      .pSpecializationInfo = nullptr, // can pass parameters to the shader
   };
   auto vert_info = VkPipelineShaderStageCreateInfo{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = VK_SHADER_STAGE_VERTEX_BIT,
      .module = this->shader_modules[1].handle,
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
   auto depth_stencil_attach_info = VkPipelineDepthStencilStateCreateInfo{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable       = VK_TRUE,
      .depthWriteEnable      = VK_TRUE,
      .depthCompareOp        = VK_COMPARE_OP_LESS, // lower depth value = closer
      .depthBoundsTestEnable = VK_FALSE, // toggle whether values outside of a depth range are culled
      .stencilTestEnable     = VK_FALSE,
      .front                 = {}, // stencil info
      .back                  = {}, // stencil info
      .minDepthBounds        = 0.0, // depth culling range
      .maxDepthBounds        = 1.0, // depth culling range
   };
   //
   // Set up push constants:
   //
   auto push_constant_struct = VkPushConstantRange{
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
      .offset     = 0,
      .size       = sizeof(push_constant),
   };
   //
   // Now let's create the pipeline layout.
   //
   auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount         = 1,
      .pSetLayouts            = &this->descriptor_set_layout.handle,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges    = &push_constant_struct,
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
      .pDepthStencilState  = &depth_stencil_attach_info,
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
}
void DovahKitVulkanSubsystem::setupFramebuffers() {
   auto& sc = this->swap_chain;
   sc.framebuffers.resize(sc.views.size());
   for (size_t i = 0; i < sc.views.size(); i++) {
      //
      // Each swap chain image needs its own view for color attachment, but they can 
      // share a single view for depth attachment because our semaphores ensure that 
      // only one subpass is running at a time.
      //
      auto attachments = std::array{ sc.views[i], this->depth_buffer.view };
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
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = indices.families.graphics,
   };
   if (vkCreateCommandPool(this->devices.logical, &pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the command pool.");
   }
}
void DovahKitVulkanSubsystem::setupDepthBuffer() {
   auto depthFormat = this->findDepthFormat();
   //
   auto& sc = this->swap_chain;
   auto& db = this->depth_buffer;
   //
   this->createVkImage(sc.extent.width, sc.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, db.image, db.memory);
   db.view = createImageView(db.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
   //
   this->transitionImageLayout(db.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
void DovahKitVulkanSubsystem::setupTextures() {
   //
   // If the "null descriptors" device feature is unavailable, then we can't write VK_NULL_HANDLE 
   // to texture descriptors. Instead, we need to create a dummy "null" texture.
   //
   if (!this->support.null_descriptors) {
      constexpr int w = 4;
      constexpr int h = 4;
      this->createVkImage(
         w, h,
         VK_FORMAT_R8G8B8A8_SRGB,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         this->null_texture.image,
         this->null_texture.memory
      );
      //
      {
         VkDeviceSize image_size = w * h * 4;
         //
         VkBuffer       staging_buffer;
         VkDeviceMemory staging_memory;
         this->createBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
         //
         void* data;
         vkMapMemory(this->devices.logical, staging_memory, 0, image_size, 0, &data);
         memset(data, 0, image_size);
         vkUnmapMemory(this->devices.logical, staging_memory);
         //
         this->transitionImageLayout(this->null_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
         this->copyBufferToImage(staging_buffer, this->null_texture.image, w, h);
         this->transitionImageLayout(this->null_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
         //
         // Discard the staging buffer:
         //
         vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
         vkFreeMemory   (this->devices.logical, staging_memory, nullptr);
      }
      //
      this->null_texture.view = this->createImageView(this->null_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
   }
   //
   auto& files = texture_files;
   auto  count = files.size();
   auto& list  = this->assets.textures;
   list.resize(count);
   for (size_t i = 0; i < count; ++i) {
      auto  name   = texture_files[i];
      auto& target = list[i];
      //
      QImage texture;
      {
         auto path      = QLatin1Literal("shaders/") + name;
         auto bytearray = QResource(path).uncompressedData();
         auto buffer    = QBuffer(&bytearray);
         buffer.open(QIODevice::ReadOnly);
         QImageReader reader(&buffer);
         if (path.endsWith("png"))
            reader.setFormat("PNG");
         else if (path.endsWith("bmp"))
            reader.setFormat("BMP");
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
         target.image,
         target.memory
      );
      //
      // Now we need to transfer our image from the staging buffer to the final buffer, 
      // transitioning its layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL as we do. We 
      // can use VK_IMAGE_LAYOUT_UNDEFINED as the "old layout" because we don't actually 
      // care about the data (or lack thereof, really) in the freshly-created VkImage.
      //
      this->transitionImageLayout(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      this->copyBufferToImage(staging_buffer, target.image, w, h);
      this->transitionImageLayout(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      //
      target.view = this->createImageView(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      // Discard the staging buffer:
      //
      vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
      vkFreeMemory   (this->devices.logical, staging_memory, nullptr);
   }
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
      .anisotropyEnable = this->support.anisotropic_filtering > 0.0 ? VK_TRUE : VK_FALSE,
      .maxAnisotropy    = std::min(8.0F, this->support.anisotropic_filtering),
      .compareEnable    = VK_FALSE,
      .compareOp        = VK_COMPARE_OP_ALWAYS,
      .minLod           = 0.0,
      .maxLod           = 0.0,
      .borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE, // true: coordinates are [0, width], etc; false: coordinates are [0, 1]
   };
   if (vkCreateSampler(this->devices.logical, &sampler_info, nullptr, &this->texture_sampler) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the texture sampler.");
   }
}
void DovahKitVulkanSubsystem::setupRenderedObjects() {
   auto& src  = models;
   auto& dst  = this->rendered_objects;
   auto  size = models.size();
   dst.resize(size);
   this->anim_state.resize(size); // TODO: decouple anim states from rendered objects eventually
   for (size_t i = 0; i < size; ++i) {
      auto& s   = src[i];
      auto& d   = dst[i];
      auto& vib = d.vertex_and_index_buffer;
      d.texture_index = i; // TODO: in the future we'd load objects and textures together, basically; for our simple test, the default 3 objects and their textures load separately
      //
      VkDeviceSize buffer_size_v = sizeof(vertex)   * s.vertices.size();
      VkDeviceSize buffer_size_i = sizeof(uint16_t) * s.indices.size();
      VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
      //
      VkBuffer       staging_buffer;
      VkDeviceMemory staging_memory;
      this->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
      //
      void* data;
      vkMapMemory(this->devices.logical, staging_memory, 0, buffer_size, 0, &data);
      memcpy((void*)((std::intptr_t)data),                 s.vertices.data(), buffer_size_v);
      memcpy((void*)((std::intptr_t)data + buffer_size_v), s.indices.data(),  buffer_size_i);
      vkUnmapMemory(this->devices.logical, staging_memory);
      //
      vib.indices_at     = buffer_size_v;
      vib.index_count    = s.indices.size();
      vib.allocated_size = buffer_size;
      //
      this->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vib.buffer, vib.memory);
      this->copyBuffer(staging_buffer, vib.buffer, buffer_size);
      d.shader_params.transform = s.transform;
      //
      vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
      vkFreeMemory   (this->devices.logical, staging_memory, nullptr);
   }
}
void DovahKitVulkanSubsystem::setupShaderParameterBuffers() {
   constexpr VkDeviceSize buffer_size = sizeof(uniform_buffer_object);
   //
   auto& sc     = this->swap_chain;
   auto  count  = sc.images.size();
   auto& list_b = sc.uniform_buffers;
   auto& list_m = sc.uniform_buffer_memory;
   list_b.resize(count);
   list_m.resize(count);
   for (size_t i = 0; i < count; i++) {
      createBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, list_b[i], list_m[i]);
   }
   //
   // Set up shader params:
   //
   constexpr VkDeviceSize rosp_buffer_size = max_rendered_objects * sizeof(rendered_object::shader_parameters);
   auto& rosp = sc.rendered_object_shader_parameters;
   rosp.buffer_handles.resize(count);
   rosp.buffer_memory.resize(count);
   for (size_t i = 0; i < count; ++i) {
      createBuffer(rosp_buffer_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, rosp.buffer_handles[i], rosp.buffer_memory[i]);
   }
}
void DovahKitVulkanSubsystem::setupDescriptorPool() {
   auto& sc = this->swap_chain;
   auto& dl = this->descriptor_set_layout;
   //
   auto image_count = sc.images.size();
   //
   std::vector<VkDescriptorPoolSize> sizes;
   for (auto& binding : dl.bindings) {
      auto count = binding.count;
      if (!binding.is_global)
         count *= image_count;
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
         .descriptorCount = count,
      });
   }
   //
   auto pool_info = VkDescriptorPoolCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets       = (uint32_t)image_count,
      .poolSizeCount = (uint32_t)sizes.size(),
      .pPoolSizes    = sizes.data(),
   };
   if (vkCreateDescriptorPool(this->devices.logical, &pool_info, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to create the descriptor pool.");
   }
}
void DovahKitVulkanSubsystem::setupDescriptorSets() {
   auto& sc = this->swap_chain;

   std::vector<VkDescriptorSetLayout> layouts(sc.images.size(), this->descriptor_set_layout.handle);
   std::vector<uint32_t> variable_counts(layouts.size(), 0);
   {  //
      // The (variable_counts) list should have one value per layout, because each layout may 
      // have only zero or one descriptor bindings with variable descriptor counts.
      //
      auto& bl = this->descriptor_set_layout.bindings;
      for (size_t i = 0; i < layouts.size(); ++i) {
         auto& bl = this->descriptor_set_layout.bindings; // TODO: if we have multiple sets per frame, pick the right set layout
         auto& vc = variable_counts[i];
         for (size_t j = 0; j < bl.size(); ++j) {
            auto& binding = bl[j];
            if (binding.flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT) {
               assert(vc == 0            && "A descriptor set is not allowed to have multiple variable-length descriptor bindings.");
               assert(j == bl.size() - 1 && "If a descriptor set has a variable-length descriptor binding, it must be the last binding in the list.");
               vc = binding.count;
            }
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
      .descriptorPool     = this->descriptor_pool,
      .descriptorSetCount = (uint32_t)layouts.size(),
      .pSetLayouts        = layouts.data(),
   };
   //
   this->descriptor_sets.resize(layouts.size());
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
   auto sampler_info = VkDescriptorImageInfo{
      .sampler     = this->texture_sampler,
      .imageView   = VK_NULL_HANDLE,
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
   };
   std::vector<VkDescriptorImageInfo> texture_infos;
   {
      auto& list = this->assets.textures;
      auto  size = list.size();
      texture_infos.resize(size);
      for (size_t i = 0; i < size; ++i) {
         texture_infos[i] = {
            .sampler     = nullptr,
            .imageView   = list[i].view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         };
      }
   }
   for (size_t i = 0; i < sc.images.size(); i++) {
      auto buffer_info = VkDescriptorBufferInfo{
         .buffer = sc.uniform_buffers[i],
         .offset = 0,
         .range  = sizeof(uniform_buffer_object), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
      };
      auto rosp_buffer_info = VkDescriptorBufferInfo{
         .buffer = sc.rendered_object_shader_parameters.buffer_handles[i],
         .offset = 0,
         .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
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
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo      = &sampler_info,
         },
         VkWriteDescriptorSet{ // storage buffer object: rendered_object::shader_parameters
            .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet           = this->descriptor_sets[i],
            .dstBinding       = 2, // this should match the binding value in the shader
            .dstArrayElement  = 0,
            .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
            .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo       = nullptr,
            .pBufferInfo      = &rosp_buffer_info,
            .pTexelBufferView = nullptr,
         },
         VkWriteDescriptorSet{ // texture array
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = this->descriptor_sets[i],
            .dstBinding      = 3, // this should match the binding value in the shader
            .dstArrayElement = 0,
            .descriptorCount = (uint32_t)texture_infos.size(),
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = texture_infos.data(),
         },
      };
      vkUpdateDescriptorSets(this->devices.logical, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
      //
      // Mark textures as synchronized:
      //
      for (auto& entry : this->assets.textures)
         entry.frame_dirty_flags &= ~(decltype(entry.frame_dirty_flags))(1 << i);
   }
}
void DovahKitVulkanSubsystem::setupCommandBuffers() {
   //
   // Command buffers allow us to "record" several draw commands (possibly across multiple 
   // threads) and then execute them all at once in the main thread. You could think of it 
   // like a transaction, I guess. A command buffer cannot receive commands while the GPU 
   // is executing any commands it's already received, which is why we use multiple command 
   // buffers for rendering (one per framebuffer).
   //
   this->command_buffers.resize(this->swap_chain.framebuffers.size());
   this->command_buffer_is_out_of_date.resize(this->command_buffers.size());
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
      this->refillCommandBuffers(i);
   }
}
void DovahKitVulkanSubsystem::setupSemaphores() {
   //
   // Semaphores are used to synchronize concurrent processes occurring within the GPU. Fences are 
   // used to synchronize the CPU with the GPU -- which is to say: waiting on a fence blocks the 
   // CPU by definition, but waiting on a semaphore allows the CPU to do work -- and send commands 
   // to the GPU to be executed when possible.
   //
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
   this->setupDepthBuffer();
   this->setupFramebuffers();
   this->setupShaderParameterBuffers();
   this->setupDescriptorPool();
   this->setupDescriptorSets();
   this->setupCommandBuffers();
   
   //
   // The above procedure will have reset all shader-side data for rendered objects, 
   // so we need to mark all rendered objects as dirty so we resynchronize that.
   //
   for (auto& ro : this->rendered_objects)
      ro.frame_dirty_flags = -1;
}

void DovahKitVulkanSubsystem::teardownSwapChain() {
   auto  device = this->devices.logical;
   auto& sc     = this->swap_chain;
   //
   auto& db = this->depth_buffer;
   vkDestroyImageView(device, db.view,   nullptr);
   vkDestroyImage    (device, db.image,  nullptr);
   vkFreeMemory      (device, db.memory, nullptr);
   //
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

void DovahKitVulkanSubsystem::refillCommandBuffers(size_t which_frame) {
   auto& command_buffer = this->command_buffers[which_frame];
   this->command_buffer_is_out_of_date[which_frame] = false;
   qDebug("Refilling command buffer for frame %u.", which_frame);
   //
   vkResetCommandBuffer(command_buffer, 0);
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
   auto clear_values = std::array{
      //
      // Values here should match the attachments we're using.
      //
      VkClearValue{ .color        = {0, 0, 0, 1} }, // color attachment uses VK_ATTACHMENT_LOAD_OP_CLEAR; this is the value to clear with
      VkClearValue{ .depthStencil = {1.0, 0} },     // depth attachment uses VK_ATTACHMENT_LOAD_OP_CLEAR; this is the depth range to celar with
   };
   auto pass_begin_info = VkRenderPassBeginInfo{
      .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass  = this->render_pass,
      .framebuffer = this->swap_chain.framebuffers[which_frame],
      .renderArea  = {
         .offset = { 0, 0 },
         .extent = this->swap_chain.extent,
      },
      .clearValueCount = (uint32_t)clear_values.size(),
      .pClearValues    = clear_values.data(),
   };

   bool any_deleted_objects = false;

   vkCmdBeginRenderPass(command_buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
   {
      using index_type = decltype(_model::indices)::value_type;
      constexpr bool indices_are_uint32_t = std::is_same_v<uint32_t, index_type>;
      constexpr bool indices_are_uint16_t = std::is_same_v<uint16_t, index_type>;
      static_assert(indices_are_uint32_t || indices_are_uint16_t);
      //
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);
      //
      {
         VkDeviceSize offset = 0;
         for (size_t j = 0; j < this->rendered_objects.size(); ++j) {
            auto& ro  = this->rendered_objects[j];
            auto& vib = ro.vertex_and_index_buffer;
            //
            if (ro.empty()) // object is deleted
               continue;
            if (ro.pending_delete) {
               any_deleted_objects = true;
               continue;
            }
            //
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &vib.buffer, &offset);
            if constexpr (indices_are_uint32_t) {
               vkCmdBindIndexBuffer(command_buffer, vib.buffer, vib.indices_at, VK_INDEX_TYPE_UINT32);
            } else if constexpr (indices_are_uint16_t) {
               vkCmdBindIndexBuffer(command_buffer, vib.buffer, vib.indices_at, VK_INDEX_TYPE_UINT16);
            }
            auto pc = push_constant{
               .texture_index = (int32_t)ro.texture_index,
               .object_index  = (int32_t)j,
            };
            vkCmdPushConstants(
               command_buffer,
               this->pipeline_layout,
               VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
               0,
               sizeof(push_constant),
               (void*)&pc
            );
            //
            // TODO: Descriptor sets will vary if the object uses different shaders, I think. For a dynamic scene, 
            // I believe we can pre-sort the rendered objects by descriptor set, and then only (re)bind descriptor 
            // sets when the current object's sets differ from those of the previous object.
            //
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 0, 1, &this->descriptor_sets[which_frame], 0, nullptr);
            //
            vkCmdDrawIndexed(command_buffer, (uint32_t)vib.index_count, 1, 0, 0, 0);
         }
         qDebug("Command buffer: drew %u objects.", this->rendered_objects.size());
      }
   }
   vkCmdEndRenderPass(command_buffer);

   if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
      throw std::runtime_error("[DovahKitVulkanSubsystem] Failed to record a command buffer.");
   }

   if (any_deleted_objects) {
      uint32_t mask = (1 << this->swap_chain.images.size()) - 1;
      //
      for (size_t i = 0; i < this->rendered_objects.size(); ++i) {
         auto& ro = this->rendered_objects[i];
         if (ro.pending_delete) {
            ro.frame_dirty_flags &= ~(1 << which_frame);
            if ((ro.frame_dirty_flags & mask) == 0) {
               this->deleteRenderedObject(i);
            }
         }
      }
   }
}

size_t DovahKitVulkanSubsystem::insertNewLoadedTexture() {
   auto& list = this->assets.textures;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.image == VK_NULL_HANDLE && !item.pending_delete)
         return i;
   }
   if (size >= max_available_textures)
      return std::string::npos;
   list.emplace_back();
   return size;
}
size_t DovahKitVulkanSubsystem::insertNewRenderedObject() {
   auto& list = this->rendered_objects;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (item.empty() && !item.pending_delete)
         return i;
   }
   if (size >= max_rendered_objects)
      return std::string::npos;
   list.emplace_back();
   this->anim_state.emplace_back(); // TODO: decouple anim states from rendered objects eventually
   return size;
}
void DovahKitVulkanSubsystem::deleteRenderedObject(size_t index) {
   qDebug("Attempting to delete rendered object #%u.", index);
   auto& list = this->rendered_objects;
   auto& ro   = list[index];
   assert(ro.pending_delete);
   {
      uint32_t mask = (1 << this->swap_chain.images.size()) - 1;
      assert((ro.frame_dirty_flags & mask) == 0 && "Do not delete rendered objects before their vertex-and-index buffers have been unhooked from all frames in flight.");
   }
   //
   auto  device = this->devices.logical;
   auto& vib    = ro.vertex_and_index_buffer;
   if (vib.buffer == VK_NULL_HANDLE) {
      assert(vib.memory == VK_NULL_HANDLE);
   } else {
      vkDestroyBuffer(device, vib.buffer, nullptr);
      vkFreeMemory   (device, vib.memory, nullptr);
      vib.buffer = VK_NULL_HANDLE;
      vib.memory = VK_NULL_HANDLE;
   }
   vib.index_count    = 0;
   vib.indices_at     = 0;
   vib.allocated_size = 0;
   //
   ro.pending_delete    = false;
   ro.frame_dirty_flags = -1;
   ro.texture_index     = -1;
   //
   qDebug("Deleted rendered object #%u.", index);
}

void DovahKitVulkanSubsystem::updateShaderParameterBuffers(uint32_t which) {
   auto& sc = this->swap_chain;
   assert(which < sc.images.size());
   //
   static auto start_time = std::chrono::high_resolution_clock::now();
   //
   auto  now     = std::chrono::high_resolution_clock::now();
   float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(now - start_time).count();
   //
   uniform_buffer_object ubo{};
   ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
   ubo.proj = glm::perspective(glm::radians(45.0f), sc.extent.width / (float)sc.extent.height, 0.1f, 10.0f);
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
   //
   // Let's also update the rendered-object shader parameters storage buffer here:
   //
   {
      using entry_type = rendered_object::shader_parameters;
      constexpr auto entry_size = sizeof(entry_type);

      constexpr bool map_only_what_is_necessary = true;

      auto& rosp   = sc.rendered_object_shader_parameters;
      auto& handle = rosp.buffer_handles[which];
      auto& memory = rosp.buffer_memory[which];
      //
      auto& ro     = this->rendered_objects;
      auto  count  = ro.size();
      assert(count < max_rendered_objects);
      VkDeviceSize size = count * entry_size;
      //
      uint32_t flag = 1 << which; // TODO: if the swap chain frame count exceeds 32, this won't work
      //
      size_t first_dirty = 0;
      size_t last_dirty  = 0;
      bool   any_dirty   = false;
      if constexpr (map_only_what_is_necessary) {
         for (size_t i = 0; i < count; ++i) {
            const auto& item = ro[i];
            if (item.pending_delete || item.empty()) // TODO: could skip the "empty" check if we force the dirty-flags to 0 on empty items and set to -1 when filling them again
               continue;
            if (item.frame_dirty_flags & flag) {
               if (!any_dirty) {
                  first_dirty = i;
                  any_dirty   = true;
               }
               last_dirty = i;
            }
         }
      } else {
         for (size_t i = 0; i < count; ++i) {
            const auto& item = ro[i];
            if (item.pending_delete || item.empty())
               continue;
            if (item.frame_dirty_flags & flag) {
               first_dirty = i;
               any_dirty   = true;
               break;
            }
         }
      }
      //
      if (any_dirty) {
         constexpr bool map_only_what_is_necessary = true;
         //
         entry_type* data = nullptr;
         if constexpr (map_only_what_is_necessary) {
            VkDeviceSize offset = first_dirty * entry_size;
            VkDeviceSize length = (last_dirty - first_dirty + 1) * entry_size;
            vkMapMemory(this->devices.logical, memory, offset, length, 0, (void**)&data);
            for (size_t i = first_dirty; i <= last_dirty; ++i) {
               auto& item = ro[i];
               if (item.pending_delete || item.empty())
                  continue;
               if (!(item.frame_dirty_flags & flag))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i - first_dirty];
               memcpy(&dst, &src, entry_size);
               //
               ro[i].frame_dirty_flags &= ~flag;
            }
         } else {
            vkMapMemory(this->devices.logical, memory, 0, size, 0, (void**)&data);
            for (size_t i = first_dirty; i < count; ++i) {
               auto& item = ro[i];
               if (item.pending_delete || item.empty())
                  continue;
               if (!(item.frame_dirty_flags & flag))
                  continue;
               auto& src = ro[i].shader_params;
               auto& dst = data[i];
               memcpy(&dst, &src, entry_size);
               //
               item.frame_dirty_flags &= ~flag;
            }
         }
         vkUnmapMemory(this->devices.logical, memory);
      }
   }
}
void DovahKitVulkanSubsystem::updateShaderTextureDescriptors(uint32_t which_frame) {
   auto&    list = this->assets.textures;
   uint32_t size = list.size();
   //
   struct pending_write {
      uint32_t start = 0;
      std::vector<VkDescriptorImageInfo> entries;
      //
      inline uint32_t end() const noexcept { return this->start + this->entries.size(); }
   };
   //
   std::vector<pending_write> writes;
   auto flag      = decltype(loaded_texture::frame_dirty_flags)(1) << which_frame;
   bool deletions = false;
   for (uint32_t i = 0; i < size; ++i) {
      auto& item = list[i];
      if (!(item.frame_dirty_flags & flag))
         continue;
      if (item.image == VK_NULL_HANDLE) // deleted texture
         continue;
      item.frame_dirty_flags &= ~flag;
      auto view = item.view;
      if (item.pending_delete) {
         if (!this->support.null_descriptors) {
            view = this->null_texture.view;
            assert(view != VK_NULL_HANDLE && "Null descriptor handles aren't supported, but we never set up our null texture!");
         } else {
            view = VK_NULL_HANDLE;
         }
         deletions = true; // TODO: optimize by only setting this if the texture has no dirty flags (that correspond to actual frames) remaining
      }
      //
      if (!writes.empty()) { // group consecutive textures into a single write, when possible
         auto& back = writes.back();
         if (back.end() == i) {
            back.entries.emplace_back(VkDescriptorImageInfo{
               .sampler     = nullptr,
               .imageView   = view,
               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            });
            continue;
         }
      }
      writes.emplace_back(pending_write{
         .start   = i,
         .entries = {
            {
               .sampler     = nullptr,
               .imageView   = view,
               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            }
         },
      });
   }
   if (writes.empty())
      return;
   //
   auto& target_set = this->descriptor_sets[which_frame];
   //
   std::vector<VkWriteDescriptorSet> write_info(writes.size());
   for (size_t i = 0; i < writes.size(); ++i) {
      auto& src  = writes[i];
      auto& info = writes[i].entries;
      //
      write_info[i] = VkWriteDescriptorSet{ // texture array
         .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet          = target_set,
         .dstBinding      = 3, // this should match the binding value in the shader
         .dstArrayElement = src.start,
         .descriptorCount = (uint32_t)info.size(),
         .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
         .pImageInfo      = info.data(),
      };
   }
   vkUpdateDescriptorSets(this->devices.logical, (uint32_t)write_info.size(), write_info.data(), 0, nullptr);
   //
   // Updating a descriptor set will invalidate any command buffers using it; they must 
   // be reset and their queue regenerated:
   //
   this->command_buffer_is_out_of_date[which_frame] = true;
   //
   if (deletions) {
      //
      // Textures were marked for delete; any textures that have been unhooked from all 
      // frames in flight should be deleted.
      //
      uint32_t mask = (uint32_t(1) << this->swap_chain.images.size()) - 1; // bits set only for valid frames
      //
      auto device = this->devices.logical;
      for (auto& item : list) {
         if (!item.pending_delete)
            continue;
         if (item.frame_dirty_flags & mask) // texture hasn't been unhooked from all frames yet
            continue;
         qDebug("Attempting to delete texture: %s", qUtf8Printable(item.path));
         assert(item.refcount == 0);
         assert(item.image    != VK_NULL_HANDLE);
         vkDestroyImageView(device, item.view,   nullptr);
         vkDestroyImage    (device, item.image,  nullptr);
         vkFreeMemory      (device, item.memory, nullptr);
         item.view   = VK_NULL_HANDLE;
         item.image  = VK_NULL_HANDLE;
         item.memory = VK_NULL_HANDLE;
         item.path.clear();
         item.w = item.h = 0;
         item.pending_delete = false;
         qDebug(" - Texture deleted.");
      }
   }
}
void DovahKitVulkanSubsystem::drawFrame() {
   this->updateAnimationState();
   //
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
   this->updateShaderParameterBuffers(imageIndex);
   this->updateShaderTextureDescriptors(imageIndex); // can invalidate command buffers, so must run before we check whether command buffers need refilling
   if (this->command_buffer_is_out_of_date[imageIndex]) {
      this->refillCommandBuffers(imageIndex);
   }
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


void DovahKitVulkanSubsystem::setAnimationPaused(size_t i, bool paused) {
   auto& as = this->anim_state;
   if (i >= as.size())
      return;
   as[i].playing = !paused;
}
void DovahKitVulkanSubsystem::updateAnimationState() {
   //
   // TODO: Arguably we may want to do this elsewhere; we should eventually separate 
   //       anim_state from the renderer core. You could imagine a tree of NiNode 
   //       structs, some with animation state, which map to a flat list of rendered 
   //       objects in the engine; and you could imagine these structs pushing updates 
   //       elsewhere in the overall flow (or at a fixed tick rate e.g. 60 FPS, etc.).
   //
   auto  now     = std::chrono::high_resolution_clock::now();
   float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(now - this->last_update).count();
   this->last_update = now;
   //
   auto& ro    = this->rendered_objects;
   auto& as    = this->anim_state;
   auto  count = ro.size(); // TODO: decouple anim state indices from rendered object indices; only store anim state for actual animated objects
   //
   for (size_t i = 0; i < count; ++i) {
      if (!as[i].playing)
         continue;
      as[i].elapsed += elapsed;
      if (as[i].elapsed > as[i].duration)
         as[i].elapsed -= as[i].duration;
      //
      auto t = ro[i].transform();
      t = glm::rotate(t, (elapsed / as[i].duration) * glm::radians(360.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      ro[i].set_transform(t);
   }
}


size_t DovahKitVulkanSubsystem::addTexture(const QString& texture_path) {
   constexpr size_t fail = std::string::npos;
   //
   auto& list = this->assets.textures;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      if (list[i].path == texture_path) {
         return i;
      }
   }
   QImage texture;
   {
      //auto path      = QLatin1Literal("shaders/") + texture_path;
      //auto bytearray = QResource(path).uncompressedData();
      auto file = QFile(texture_path);
      if (!file.open(QIODevice::ReadOnly)) {
         qDebug("[DovahKitVulkanSubsystem][addRenderedObject] Failed to open test image.");
         return fail;
      }
      auto bytearray = file.readAll();
      auto buffer    = QBuffer(&bytearray);
      buffer.open(QIODevice::ReadOnly);
      QImageReader reader(&buffer);
      if (texture_path.endsWith("png"))
         reader.setFormat("PNG");
      else if (texture_path.endsWith("bmp"))
         reader.setFormat("BMP");
      reader.read(&texture);
      texture = texture.convertToFormat(QImage::Format::Format_RGBA8888);
   }
   if (texture.isNull()) {
      qDebug("[DovahKitVulkanSubsystem][addRenderedObject] Failed to load test image.");
      return fail;
   }
   uint32_t w = texture.width();
   uint32_t h = texture.height();
   //
   // here, we may want to lock the texture asset list, if we were doing a multithreaded renderer
   //
   auto texture_index = this->insertNewLoadedTexture();
   if (texture_index == std::string::npos) {
      qDebug("Cannot add new rendered textures. Maximum has been reached.");
      return fail;
   }
   auto& target = list[texture_index];
   target.w    = w;
   target.h    = h;
   target.path = texture_path;
   //
   VkDeviceSize image_size = w * h * 4;
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
      target.image,
      target.memory
   );
   //
   // Now we need to transfer our image from the staging buffer to the final buffer, 
   // transitioning its layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL as we do. We 
   // can use VK_IMAGE_LAYOUT_UNDEFINED as the "old layout" because we don't actually 
   // care about the data (or lack thereof, really) in the freshly-created VkImage.
   //
   this->transitionImageLayout(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
   this->copyBufferToImage(staging_buffer, target.image, w, h);
   this->transitionImageLayout(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
   //
   target.view = createImageView(target.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
   //
   // Discard the staging buffer:
   //
   vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
   vkFreeMemory   (this->devices.logical, staging_memory, nullptr);
   //
   target.frame_dirty_flags = -1;
   return texture_index;
}
void DovahKitVulkanSubsystem::addRenderedObject(const QString& texture_path) {
   size_t texture_index = this->addTexture(texture_path);
   if (texture_index == std::string::npos) {
      qDebug("Cannot add new rendered object: failed to add its texture.");
      return;
   }
   auto&  texture_item = this->assets.textures[texture_index];
   size_t object_index = this->insertNewRenderedObject();
   if (object_index == std::string::npos) {
      qDebug("Cannot add new rendered objects. Maximum has been reached.");
      if (texture_item.refcount == 0) {
         //
         // This texture was created for us, but we never got a chance to use it. Mark it 
         // for deletion.
         //
         texture_item.pending_delete    = true;
         texture_item.frame_dirty_flags = -1;
      }
      return;
   }
   QSize texture_size = { (int)texture_item.w, (int)texture_item.h }; // just used to size the quad so we maintain aspect ratio
   {  // Create model
      if (!texture_size.isValid())
         texture_size = { 1, 1 };
      //
      auto& ro  = this->rendered_objects[object_index];
      auto& vib = ro.vertex_and_index_buffer;
      ro.texture_index = texture_index;
      ++texture_item.refcount;
      texture_item.pending_delete = false;
      //
      glm::vec3 position = {};
      for (size_t j = 0; j < 3; ++j)
         position[j] = ((float)rand() / RAND_MAX) * 5.0F - 2.5F;
      //
      float hfwc = ((float)texture_size.height() / texture_size.width()) / 2; // height-for-width, centered
      std::array<vertex, 4> vertices = {
         vertex{ { -0.5f, -hfwc, 0.0 }, { 1.0f, 0.0f, 0.0f }, { 1.0, 0.0 } },
         vertex{ {  0.5f, -hfwc, 0.0 }, { 0.0f, 1.0f, 0.0f }, { 0.0, 0.0 } },
         vertex{ {  0.5f,  hfwc, 0.0 }, { 0.0f, 0.0f, 1.0f }, { 0.0, 1.0 } },
         vertex{ { -0.5f,  hfwc, 0.0 }, { 1.0f, 1.0f, 1.0f }, { 1.0, 1.0 } },
      };
      std::array<uint16_t, 6> indices = { 0, 1, 2, 2, 3, 0 };
      glm::mat4 transform = glm::translate(
         glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
         position
      );
      //
      VkDeviceSize buffer_size_v = sizeof(vertex)   * vertices.size();
      VkDeviceSize buffer_size_i = sizeof(uint16_t) * indices.size();
      VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
      //
      VkBuffer       staging_buffer;
      VkDeviceMemory staging_memory;
      this->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_memory);
      //
      void* data;
      vkMapMemory(this->devices.logical, staging_memory, 0, buffer_size, 0, &data);
      memcpy((void*)((std::intptr_t)data), vertices.data(), buffer_size_v);
      memcpy((void*)((std::intptr_t)data + buffer_size_v), indices.data(), buffer_size_i);
      vkUnmapMemory(this->devices.logical, staging_memory);
      //
      vib.indices_at     = buffer_size_v;
      vib.index_count    = indices.size();
      vib.allocated_size = buffer_size;
      //
      this->createBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vib.buffer, vib.memory);
      this->copyBuffer(staging_buffer, vib.buffer, buffer_size);
      ro.shader_params.transform = transform;
      //
      vkDestroyBuffer(this->devices.logical, staging_buffer, nullptr);
      vkFreeMemory(this->devices.logical, staging_memory, nullptr);
      //
      ro.frame_dirty_flags = -1;
   }
   //
   for (size_t i = 0; i < this->command_buffer_is_out_of_date.size(); ++i) // range-based for-loops are broken for std::vector<bool>
      this->command_buffer_is_out_of_date[i] = true;
}
void DovahKitVulkanSubsystem::removeRenderedObject() {
   auto& list = this->rendered_objects;
   if (list.empty())
      return;
   std::decay_t<decltype(list)>::reverse_iterator it;
   for (it = list.rbegin(); it != list.rend(); ++it) {
      auto& item = *it;
      if (item.empty() || item.pending_delete)
         continue;
      break;
   }
   if (it == list.rend())
      return;
   auto& item = *it;
   item.pending_delete    = true;
   item.frame_dirty_flags = -1;
   {
      auto ti = item.texture_index;
      if (ti >= 0) {
         auto& list = this->assets.textures;
         if (ti < list.size()) {
            auto& tex = list[ti];
            if (--tex.refcount == 0) {
               tex.pending_delete    = true;
               tex.frame_dirty_flags = -1;
            }
         }
      }
   }
   item.texture_index = -1;
   //
   for (size_t i = 0; i < this->command_buffer_is_out_of_date.size(); ++i) // range-based for-loops are broken for std::vector<bool>
      this->command_buffer_is_out_of_date[i] = true;
}