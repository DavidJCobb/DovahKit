#include "surface_renderer.h"
#include <QResource> // for loading shaders
#include "DKVulkanInstance.h"
#include "frame_in_flight.h"
#include "material.h"
#include "physical_device.h"
#include "queue_family_info.h"
#include "render_pass.h"
#include "shader_module.h"
#include "vertex.h"
#include "config/frames_in_flight.h"
#include "config/scene_limits.h"
#include "config/validation_layers.h"

// loading textures from files using Qt:
#include <QBuffer>
#include <QImage>
#include <QImageReader>

#include "loaded_texture.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
//
#include <QFile>
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
   };

   static constexpr auto desired_swap_chain_presentation_mode  = VK_PRESENT_MODE_MAILBOX_KHR;
   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;
}

namespace { // test scene properties
   struct _model {
      using vertex = vulkanDK::vertex;
      const std::vector<vertex>   vertices;
      const std::vector<uint16_t> indices;
      glm::mat4 transform;
   };

   std::array initial_textures = {
      "Tamriel-Skyrim.esm.png",
      "ScreenShot278.bmp",
      "ScreenShot389.bmp",
   };

   std::array initial_meshes = {
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

namespace vulkanDK {
   #pragma region communicate with DKVulkanView
   // renderer events:
   void surface_renderer::_on_renderer_ready() {
      if (this->widget.pointer)
         emit this->widget.pointer->rendererReady();
   }
   void surface_renderer::_on_renderer_teardown_imminent() {
      if (this->widget.pointer)
         emit this->widget.pointer->rendererTeardownImminent();
   }
   void surface_renderer::_on_renderer_teardown_complete() {
      if (this->widget.pointer)
         emit this->widget.pointer->rendererTeardownComplete();
   }

   // widget events:
   void surface_renderer::_on_repaint() {
      this->draw_next_frame();
   }
   void surface_renderer::_on_visibility_change(QSize size, bool visible) {
      this->widget.resized = true;
      this->widget.visible = visible && !size.isEmpty();
   }
   #pragma endregion

   #pragma region expose to DKVulkanView
   void surface_renderer::set_physical_device(const physical_device& pd) {
      if (this->logical_device != VK_NULL_HANDLE) {
         this->teardown();
      }
      this->device_info = &pd;
      if (this->handle != VK_NULL_HANDLE) {
         this->_init_device();
         this->setup();
      }
   }
   void surface_renderer::set_widget(DKVulkanView* widget) {
      if (this->widget.pointer == widget)
         return;
      if (this->handle != VK_NULL_HANDLE) {
         this->teardown();
         this->_reset_surface();
      }
      this->widget.pointer = widget;
      if (widget) {
         this->widget.last_id = widget->winId();
         this->_init_surface();
         if (this->device_info) { // do we know what physical device we want to use yet?
            this->_init_device();
            this->setup();
         }
         this->widget.resized = false;
         this->widget.visible = widget->isVisible();
      } else {
         this->widget.last_id = {};
         this->widget.resized = false;
         this->widget.visible = false;
      }
   }
   void surface_renderer::update_widget_id() {
      if (this->widget.pointer == nullptr)
         return;
      auto id = this->widget.pointer->winId();
      if (id == this->widget.last_id)
         return;
      this->teardown();
      this->_reset_surface();
      this->widget.last_id = id;
      this->_init_surface();
      if (this->device_info) { // do we know what physical device we want to use yet?
         this->_init_device();
         this->setup();
      }
   }
   #pragma endregion

   surface_renderer::surface_renderer(DKVulkanInstance& dkvi, DKVulkanView* widget) : owner(dkvi), null_texture(*this) {
      this->descriptor_set_layouts.resize(1);
      this->descriptor_set_layouts[0].bindings = {
         vulkanDK::descriptor_binding{ // uniform buffer object
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // texture sampler
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_SAMPLER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_object::shader_parameters
            .index              = 2,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // texture array
            .index              = 3,
            .flags              = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            .type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .count              = config::max_loaded_textures,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      //
      this->set_widget(widget);
      if (this->handle == VK_NULL_HANDLE) {
         qDebug("[surface_renderer] Failed to initialize surface (constructor).");
         return;
      }
   }
   surface_renderer::~surface_renderer() {
      this->teardown();
      this->_reset_surface();
   }

   void surface_renderer::_init_surface() {
      auto create_info = VkWin32SurfaceCreateInfoKHR{
         .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
         .hinstance = GetModuleHandle(nullptr),
         .hwnd      = (HWND)this->widget.last_id,
      };
      if (vkCreateWin32SurfaceKHR(this->owner.getHandle(), &create_info, nullptr, &this->handle) != VK_SUCCESS) {
         this->handle         = VK_NULL_HANDLE;
         this->widget.last_id = {};
      }
   }
   void surface_renderer::_init_device() {
      auto  pd_handle  = this->device_info->handle;
      auto& pd_support = this->device_info->support;
      //
      auto  indices        = queue_family_info(*this->device_info, this->handle);
      float queue_priority = 1.0F;
      std::vector<VkDeviceQueueCreateInfo> queue_infos;
      {
         queue_infos.reserve(queue_family_info::unique_family_count);
         //
         for (size_t i = 0; i < queue_family_info::unique_family_count; ++i) {
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
         .samplerAnisotropy = pd_support.max_anisotropic_filtering > 0 ? VK_TRUE : VK_FALSE,
      };
      //
      auto robustness_extensions = VkPhysicalDeviceRobustness2FeaturesEXT{
         .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
         .pNext               = nullptr,
         .robustBufferAccess2 = VK_FALSE,
         .robustImageAccess2  = VK_FALSE,
         .nullDescriptor      = pd_support.descriptor_bindings.null_handles ? VK_TRUE : VK_FALSE,
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
      if (config::enable_validation_layers) {
         create_info.enabledLayerCount   = static_cast<uint32_t>(config::desired_validation_layers.size());
         create_info.ppEnabledLayerNames = config::desired_validation_layers.data();
      } else {
         create_info.enabledLayerCount = 0;
      }
      //
      if (vkCreateDevice(pd_handle, &create_info, nullptr, &this->logical_device) != VK_SUCCESS) {
         // report VK_ERROR_DEVICE_LOST
         throw std::runtime_error("[vulkanDK::surface_renderer] Failed to create logical device.");
      }
      //
      // And lastly, let's get our queues:
      //
      this->queues.graphics.setup    (this->logical_device, indices.families.graphics);
      this->queues.presentation.setup(this->logical_device, indices.families.presentation);
   }

   void surface_renderer::_reset_surface() {
      if (this->handle != VK_NULL_HANDLE) {
         vkDestroySurfaceKHR(this->owner.getHandle(), this->handle, nullptr);
         this->handle = VK_NULL_HANDLE;
      }
   }

   void surface_renderer::setup() {
      if (this->logical_device == VK_NULL_HANDLE) {
         return;
      }
      this->configuration.image_count = config::frames_in_flight_count; // TODO: use swap chain size instead?
      this->swap_chain.frames_in_flight.resize(config::frames_in_flight_count);
      //
      this->setup_descriptor_set_layouts();
      this->_setup_shader_modules();
      this->setup_texture_sampler(); // descriptor set layout must be able to refer to our immutable sampler
      //
      this->setup_command_pool(this->queues.graphics.index);
      this->setup_descriptor_pool();
      //
      {  // swap chain
         this->_setup_swap_chain_instance();
         this->_setup_render_passes(); // requires swap chain format
         this->_setup_materials(); // requires render pass
         this->_setup_depth_buffer();
         this->_setup_swap_chain_images();
         this->_setup_framebuffers();
         {  // frames in flight (semaphores, shader parameter buffers, descriptor set allocations, command buffers)
            auto& list = this->swap_chain.frames_in_flight;
            auto  size = list.size();
            for (size_t i = 0; i < size; ++i)
               list[i].setup(*this, i);
         }
      }
      this->_create_null_texture();
      this->_setup_initial_scene();
      this->_initialize_descriptor_sets();
      //
      this->_on_renderer_ready();
   }
   void surface_renderer::_setup_shader_modules() {
      auto& dfn = this->material_definitions.emplace_back();
      auto vert_binding    = vertex::getBindingDescription();
      auto vert_attributes = vertex::getAttributeDescriptions();
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(this->logical_device, QResource("shaders/shader.frag.spv").uncompressedData());
         vert = new shader_module(this->logical_device, QResource("shaders/shader.vert.spv").uncompressedData());
         this->shader_modules.push_back(frag);
         this->shader_modules.push_back(vert);
      }
      if (frag->empty()) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_shader_modules] Failed to load fragment shader.");
      }
      if (vert->empty()) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_shader_modules] Failed to load vertex shader.");
      }
      //
      dfn.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
            .specialization_info = nullptr,
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
            .specialization_info = nullptr,
         },
      };
      dfn.color_blending.blends.emplace_back(material_definition::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
      {
         auto& vertex     = dfn.inputs.vertex;
         auto  attributes = vertex::getAttributeDescriptions();
         vertex.bindings.push_back(vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
   }
   //
   void surface_renderer::_create_null_texture() {
      if (this->device_info->support.descriptor_bindings.null_handles)
         return;
      auto& nt = this->null_texture;
      //
      constexpr int w = 4;
      constexpr int h = 4;
      nt.create_image(
         w, h,
         VK_FORMAT_R8G8B8A8_SRGB,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      //
      {
         VkDeviceSize image_size = w * h * 4;
         //
         auto staging = this->create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         //
         void* data = staging.map_memory();
         memset(data, 0, image_size);
         staging.unmap_memory(data);
         //
         nt.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
         nt.copy_content_from_buffer(staging.handle);
         nt.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      }
      //
      nt.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
   }
   void surface_renderer::_setup_initial_scene() {
      auto device = this->logical_device;
      //
      // Textures:
      //
      {
         auto& list_src = initial_textures;
         auto& list_dst = this->scene.textures;
         auto  count    = list_src.size();
         list_dst.resize(count);
         for (size_t i = 0; i < count; ++i) {
            auto  name   = list_src[i];
            auto& target = list_dst[i];
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
            auto staging = this->create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            //
            void* data = staging.map_memory();
            memcpy(data, texture.constBits(), image_size);
            staging.unmap_memory(data);
            //
            uint32_t w = texture.width();
            uint32_t h = texture.height();
            texture = QImage();
            //
            // Now let's create an image:
            //
            target.content = concrete_image(*this);
            target.content.create_image(
               w, h,
               VK_FORMAT_R8G8B8A8_SRGB,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
            //
            // Now we need to transfer our image from the staging buffer to the final buffer, 
            // transitioning its layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL as we do. We 
            // can use VK_IMAGE_LAYOUT_UNDEFINED as the "old layout" because we don't actually 
            // care about the data (or lack thereof, really) in the freshly-created VkImage.
            //
            target.content.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            target.content.copy_content_from_buffer(staging.handle);
            target.content.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            target.content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
         }
      }
      //
      // Meshes:
      //
      {
         auto& list_src = initial_meshes;
         auto& list_dst = this->scene.meshes;
         auto  count    = list_src.size();
         list_dst.resize(count);
         for (size_t i = 0; i < count; ++i) {
            auto& s   = list_src[i];
            auto& d   = list_dst[i];
            auto& vib = d.vertex_and_index_buffer;
            d.texture_index = i; // TODO: in the future we'd load objects and textures together, basically; for our simple test, the default 3 objects and their textures load separately
            {
               d.anim_state = new mesh_animation_state;
            }
            //
            VkDeviceSize buffer_size_v = sizeof(vertex)   * s.vertices.size();
            VkDeviceSize buffer_size_i = sizeof(uint16_t) * s.indices.size();
            VkDeviceSize buffer_size   = buffer_size_v + buffer_size_i;
            //
            auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data    = staging.map_memory();
            memcpy((void*)((std::intptr_t)data),                 s.vertices.data(), buffer_size_v);
            memcpy((void*)((std::intptr_t)data + buffer_size_v), s.indices.data(),  buffer_size_i);
            staging.unmap_memory(data);
            //
            vib.indices_at  = buffer_size_v;
            vib.index_count = s.indices.size();
            vib.buffer      = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vib.buffer.copy_from(staging);
            //
            d.shader_params.transform = s.transform;
            d.frame_dirty_flags.set_all();
         }
      }
      //
      // Done.
      //
   }
   void surface_renderer::_initialize_descriptor_sets() {
      auto sampler_info = VkDescriptorImageInfo{
         .sampler     = this->texture_sampler,
         .imageView   = VK_NULL_HANDLE,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      std::vector<VkDescriptorImageInfo> texture_infos;
      {
         auto& list = this->scene.textures;
         auto  size = list.size();
         texture_infos.resize(size);
         for (size_t i = 0; i < size; ++i) {
            texture_infos[i] = {
               .sampler     = nullptr,
               .imageView   = list[i].content.view,
               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
         }
      }
      //
      auto& sc  = this->swap_chain;
      auto& fif = sc.frames_in_flight;
      for (size_t i = 0; i < fif.size(); ++i) {
         auto& frame = fif[i];
         //
         auto buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.uniform.handle,
            .offset = 0,
            .range  = sizeof(scene_global_state), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto rosp_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.object_data.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         //
         auto descriptor_writes = std::array{
            VkWriteDescriptorSet{ // uniform buffer object
               .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .dstSet           = frame.descriptor_sets[0],
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
               .dstSet          = frame.descriptor_sets[0],
               .dstBinding      = 1, // this should match the binding value in the shader
               .dstArrayElement = 0,
               .descriptorCount = 1,
               .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
               .pImageInfo      = &sampler_info,
            },
            VkWriteDescriptorSet{ // storage buffer object: rendered_object::shader_parameters
               .sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .dstSet           = frame.descriptor_sets[0],
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
               .dstSet          = frame.descriptor_sets[0],
               .dstBinding      = 3, // this should match the binding value in the shader
               .dstArrayElement = 0,
               .descriptorCount = (uint32_t)texture_infos.size(),
               .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
               .pImageInfo      = texture_infos.data(),
            },
         };
         vkUpdateDescriptorSets(this->logical_device, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
      }
      //
      // Mark textures as synchronized:
      //
      for (auto& entry : this->scene.textures)
         entry.frame_dirty_flags.clear_all();
   }
   //
   void surface_renderer::_setup_render_passes() {
      if (this->render_passes.empty()) {
         //
         // Set up our render pass definitions.
         //
         auto* rp = new render_pass(*this);
         this->render_passes = { rp };
         //
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
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
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
         };
         rp->subpasses.descriptions = {
            {  // subpass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 1,
                     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpasses.dependencies = {
            //
            // A subpass dependency specifies that  certain tasks in the "source" must complete 
            // before other  tasks in the  "destination" are allowed  to proceed.  The "source" 
            // subpass must always precede (have a lower index than) the "destination" subpass. 
            // The special subpass index VK_SUBPASS_EXTERNAL  refers to tasks occurring outside 
            // of the render pass --  the start (source) or end (destination) of a render pass, 
            // as it were.
            // 
            // For a subpass dependency,  a "task" is an operation (access mask) and the stages 
            // in which that operation occurs (stage mask).
            // 
            // The start of a subpass  has an implicit  task: transitioning  the target image's 
            // current layout  to the one specified by the relevant attachment's  initialLayout 
            // field above. We of course need to ensure that the image in question (typically a 
            // swap chain image) is actually available (i.e. has been acquired) before any such 
            // transition is attempted.
            //
            VkSubpassDependency{
               //
               // Ensure swap chain image has been acquired.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            //
            // If you don't specify a final external dependency  -- that is, a dependency whose 
            // destination  is VK_SUBPASS_EXTERNAL  -- then  Vulkan will inject one  with these 
            // settings:
            // 
            //    .srcSubpass      = /* based on the last subpass */,
            //    .dstSubpass      = VK_SUBPASS_EXTERNAL,
            //    .srcStageMask    = /* based on the last subpass */,
            //    .dstStageMask    = VK_PIPELINE_STAGE_NONE_KHR,
            //    .srcAccessMask   = /* based on the last subpass */,
            //    .dstAccessMask   = VK_ACCESS_NONE_KHR,
            //    .dependencyFlags = 0, // guessed
            //
            // Typically, if  an attachment's  finalLayout  (specified above)  differs from the 
            // layout  that the attachment has at the  end of your last subpass, you  will need 
            // to specify  your own final  external dependency;  the default one  won't be good 
            // enough. If you're able to rely on semaphores,  though, then the default can work 
            // even in that case.
            //
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               //.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // wait until end of pipeline
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      //
      // (Re)create the render passes within the GPU:
      //
      for(auto* rp : this->render_passes)
         rp->setup();
   }
   //
   // Swap chain:
   //
   void surface_renderer::_setup_swap_chain_instance() {
      auto& sc  = this->swap_chain;
      auto  ssi = surface_support_info(*this);
      //
      VkSurfaceFormatKHR surfaceFormat;
      VkPresentModeKHR   presentMode;
      VkExtent2D         extent;
      uint32_t           imageCount;
      //
      #pragma region choose format
         assert(!ssi.formats.empty());
         surfaceFormat = ssi.formats[0]; // fallback
         for (const auto& current_format : ssi.formats) {
            if (current_format.format == VK_FORMAT_B8G8R8A8_SRGB && current_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
               surfaceFormat = current_format;
               break;
            }
         }
         sc.format = surfaceFormat.format;
      #pragma endregion
      #pragma region choose presentation mode
         presentMode = VK_PRESENT_MODE_FIFO_KHR; // fallback
         for (const auto& current_mode : ssi.presentation_modes) {
            if (current_mode == desired_swap_chain_presentation_mode) {
               presentMode = current_mode;
               break;
            }
         }
      #pragma endregion
      #pragma region choose extent
         if (ssi.capabilities.currentExtent.width != UINT32_MAX) {
            extent = ssi.capabilities.currentExtent;
         } else {
            auto& min_e = ssi.capabilities.minImageExtent;
            auto& max_e = ssi.capabilities.maxImageExtent;
            //
            extent = this->desired_surface_size();
            extent.width  = std::clamp(extent.width,  min_e.width,  max_e.width);
            extent.height = std::clamp(extent.height, min_e.height, max_e.height);
         }
         this->surface_extent = extent;
      #pragma endregion
      #pragma region choose image count
         imageCount = ssi.capabilities.minImageCount + 1;
         if (ssi.capabilities.maxImageCount > 0 && imageCount > ssi.capabilities.maxImageCount) {
            imageCount = ssi.capabilities.maxImageCount;
         }
      #pragma endregion
      //
      auto create_info = VkSwapchainCreateInfoKHR{
         .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
         .surface          = this->handle,
         .minImageCount    = imageCount,
         .imageFormat      = surfaceFormat.format,
         .imageColorSpace  = surfaceFormat.colorSpace,
         .imageExtent      = extent,
         .imageArrayLayers = 1,
         .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      };
      //
      std::array<uint32_t, 2> queue_family_indices = { this->queues.graphics.index, this->queues.presentation.index };
      if (this->queues.graphics.index != this->queues.presentation.index) {
         //
         // TODO: Apparently "exclusive" is faster for this case, but requires more complicated setup, 
         //       which the tutorial I'm following feels should be saved for later.
         // 
         // See: https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain#page_Creating-the-swap-chain
         //
         create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
         create_info.queueFamilyIndexCount = (uint32_t)queue_family_indices.size();
         create_info.pQueueFamilyIndices   = queue_family_indices.data();
      } else {
         create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
         create_info.queueFamilyIndexCount = 0;       // clearing these two values is optional, but feels cleaner to me
         create_info.pQueueFamilyIndices   = nullptr; //
      }
      create_info.preTransform   = ssi.capabilities.currentTransform; // don't rotate or otherwise transform the image while rendering
      create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // disable alpha
      create_info.presentMode    = presentMode;
      create_info.clipped        = VK_TRUE;        // disable rendering of pixels covered (e.g. by other windows); good optimization, but prevents querying the colors of those pixels (e.g. for saving snapshots)
      create_info.oldSwapchain   = VK_NULL_HANDLE; // must be specified when rebuilding a swap chain; keep null for making a new swap chain
      //
      if (vkCreateSwapchainKHR(this->logical_device, &create_info, nullptr, &sc.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_swap_chain_instance] Failed to create swap chain.");
      }
   }
   void surface_renderer::_setup_materials() {
      auto& sc = this->swap_chain;
      if (sc.materials.empty()) {
         //
         // Setting up the materials' pipeline layouts can be done at any point after we 
         // have material definitions loaded and have created handles for our descriptor 
         // set layouts.
         //
         sc.materials.resize(1);
         //
         sc.materials[0].owner = this;
         sc.materials[0].setup_layout(
            {  // Descriptor set layouts
               this->descriptor_set_layouts[0].handle,
            },
            {  // Push constants
               VkPushConstantRange{
                  .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                  .offset     = 0,
                  .size       = sizeof(rendered_mesh::push_constant),
               }
            }
         );
      }
      //
      // Setting up materials' pipeline handles requires knowledge of the final image size 
      // to render to, and so must be re-done every time we rebuild our swap chain in 
      // response to a resize.
      //
      auto viewport = VkViewport{ // describe what part of the framebuffer we should draw to
         .x        = 0.0,
         .y        = 0.0,
         .width    = (float)this->surface_extent.width,
         .height   = (float)this->surface_extent.height,
         .minDepth = 0.0, // must be >= 0
         .maxDepth = 1.0, // must be <= 1
      };
      auto scissor = VkRect2D{ // describe what part of the framebuffer we should retain (like a write-mask)
         .offset = {0, 0},
         .extent = this->surface_extent,
      };
      //
      sc.materials[0].setup_handle(this->material_definitions[0], viewport, scissor, this->render_passes[0]->handle, 0);
   }
   void surface_renderer::_setup_depth_buffer() {
      auto  extent = this->surface_extent;
      auto  format = this->find_depth_format();
      auto& db     = swap_chain.depth_buffer;
      db = concrete_image(*this);
      db.create_image(extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      db.create_basic_view(format, VK_IMAGE_ASPECT_DEPTH_BIT);
      db.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
   }
   void surface_renderer::_setup_swap_chain_images() {
      auto& sc = this->swap_chain;
      //
      uint32_t image_count;
      //
      // Get the number of swapchain images:
      //
      vkGetSwapchainImagesKHR(this->logical_device, sc.handle, &image_count, nullptr);
      if (sc.images.size() != image_count) {
         sc.images_in_flight.resize(image_count);
         sc.images.resize(image_count);
      } else {
         assert(sc.images_in_flight.size() == image_count);
      }
      //
      // Get the image handles:
      //
      std::vector<VkImage> image_handles(image_count);
      vkGetSwapchainImagesKHR(this->logical_device, sc.handle, &image_count, image_handles.data());
      for (size_t i = 0; i < image_count; ++i) {
         sc.images[i] = surface_renderer_image_view(*this, image_handles[i]);
      }
      //
      // Set up the image views:
      //
      for (size_t i = 0; i < image_count; ++i) {
         sc.images[i].create_basic_view(sc.format, VK_IMAGE_ASPECT_COLOR_BIT);
      }
   }
   void surface_renderer::_setup_framebuffers() {
      auto& sc = this->swap_chain;
      auto extent = this->surface_extent;
      auto r_pass = this->render_passes[0]->handle;
      //
      auto count = sc.images.size();
      sc.framebuffers.resize(count);
      for (size_t i = 0; i < count; i++) {
         //
         // Each swap chain image needs its own view for color attachment, but they can 
         // share a single view for depth attachment because our semaphores ensure that 
         // only one subpass is running at a time.
         //
         auto attachments = std::array{ sc.images[i].view, sc.depth_buffer.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = r_pass,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1,
         };
         if (vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &sc.framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::surface_renderer::_setup_framebuffers] Failed to create a framebuffer.");
         }
      }
   }

   void surface_renderer::teardown() {
      this->_on_renderer_teardown_imminent();
      //
      vkDeviceWaitIdle(this->logical_device); // wait for all draw commands to finish (remember: they're asynch)
      //
      // Ensure all child objects belonging to the instance are destroyed.
      //
      this->scene.teardown();
      this->null_texture.teardown();
      {  // Swap chain
         auto& sc = this->swap_chain;
         //
         sc.materials.clear();
         for (auto& fb : sc.framebuffers) {
            vkDestroyFramebuffer(this->logical_device, fb, nullptr);
            fb = VK_NULL_HANDLE;
         }
         sc.images.clear();
         sc.depth_buffer.teardown();
         vkDestroySwapchainKHR(this->logical_device, sc.handle, nullptr);
         sc.handle = VK_NULL_HANDLE;
         //
         sc.frames_in_flight.clear();
      }
      abstract_renderer::teardown(); // tears down the logical device, too
      //
      this->_on_renderer_teardown_complete();
   }

   void surface_renderer::handle_resize() {
      auto  device = this->logical_device;
      auto& sc     = this->swap_chain;
      //
      vkDeviceWaitIdle(device); // wait for all pending GPU-side commands to finish
      //
      VkFormat sc_format = sc.format;
      {  // Tear down swap chain state
         for (auto& m : sc.materials)
            //
            // We don't need to completely destroy materials including their pipeline layouts; we 
            // just need to destroy the pipelines themselves.
            //
            m.teardown_handle();
         //
         for (auto& fb : sc.framebuffers) {
            vkDestroyFramebuffer(this->logical_device, fb, nullptr);
            fb = VK_NULL_HANDLE;
         }
         for (auto& image : sc.images) {
            image.destroy_view();
            image.image = VK_NULL_HANDLE;
         }
         for (auto& handle : sc.images_in_flight)
            handle = VK_NULL_HANDLE;
         sc.depth_buffer.teardown();
         vkDestroySwapchainKHR(this->logical_device, sc.handle, nullptr);
         sc.handle = VK_NULL_HANDLE;
      }
      {  // Set up new state
         this->_setup_swap_chain_instance();
         //
         // If we have any resources that are per swap chain image, we'd want to tear them down and 
         // rebuild them here if the swap chain image count has changed. For example, if we decided 
         // to have our descriptor sets exist per swap chain image,  we'd need to teardown and then 
         // rebuild the descriptor pool here.
         //
         if (sc.format != sc_format) {
            //
            // The swap chain image format has changed. We need to update our render pass.
            // 
            // NOTE: Remember to stay in synch with _setup_render_passes()!
            //
            auto* rp = this->render_passes[0];
            assert(rp);
            rp->teardown();
            rp->attachments[0].format = sc.format;
            rp->setup();
         }
         this->_setup_materials(); // requires render pass
         this->_setup_depth_buffer();
         this->_setup_swap_chain_images();
         this->_setup_framebuffers();
         for (auto& fif : sc.frames_in_flight) {
            fif.invalidate_all_command_buffers(); // FIF doesn't have any other state that we'd need to reset
         }
         // Rebuilding the descriptor pool is only necessary if the frame-in-flight count has changed.
         this->_initialize_descriptor_sets();
         //
         sc.current_frame = 0;
      }
      //
      // The above procedure will have reset all shader-side data for rendered objects, 
      // so we need to mark all rendered objects as dirty so we resynchronize that. We 
      // don't have to update descriptors the same way because we just took care of them 
      // when initializing descriptor sets.
      //
      for (auto& ro : this->scene.meshes)
         ro.frame_dirty_flags.set_all();
      //
      // Update surface state:
      //
      this->widget.resized = false;
   }


   void surface_renderer::draw_next_frame() {
      this->scene.update();
      //
      constexpr auto no_timeout = UINT64_MAX;
      auto& sc = this->swap_chain;
      //
      //  - Acquire an image from the swap chain
      //  - Execute the command buffer with that image as attachment in the framebuffer
      //  - Return the image to the swap chain for presentation
      // 
      // These tasks are asynchronous, but must run sequentially.
      //
      if (!this->widget.visible)
         return;
      //
      // If this frame-in-flight is still being used to render and present another swap 
      // chain image, wait for it to finish. We'll also advance the current frame counter 
      // here.
      //
      auto& frame = sc.frames_in_flight[sc.current_frame];
      sc.current_frame = (sc.current_frame + 1) % sc.frames_in_flight.size();
      vkWaitForFences(this->logical_device, 1, &frame.fence, VK_TRUE, no_timeout);
      //
      // Next, let's acquire a swap chain image to use for this frame:
      //
      uint32_t sc_image_index;
      auto     result = vkAcquireNextImageKHR(this->logical_device, sc.handle, no_timeout, frame.semaphores.image_available, VK_NULL_HANDLE, &sc_image_index);
      switch (result) {
         case VK_SUCCESS:
            break;
         case VK_SUBOPTIMAL_KHR:
            if constexpr (rebuild_swap_chain_asap_if_suboptimal) {
               this->handle_resize();
               return;
            }
            break;
         case VK_ERROR_OUT_OF_DATE_KHR:
            this->handle_resize();
            return;
         default:
            throw std::runtime_error("[vulkanDK::surface_renderer::draw_next_frame] Failed to acquire swap chain image!");
      }
      //
      {
         auto& handle = sc.images_in_flight[sc_image_index];
         //
         // Check if a previous frame is using this image.
         //
         if (handle != VK_NULL_HANDLE) {
            vkWaitForFences(this->logical_device, 1, &handle, VK_TRUE, UINT64_MAX);
         }
         //
         // Mark the image as now being in use by this frame.
         //
         handle = frame.fence;
      }
      //
      // Render the image:
      //
      frame.draw(sc.framebuffers[sc_image_index]);
      //
      // Present the image:
      //
      auto signal_semaphores  = std::array{ frame.semaphores.render_finished };
      auto swap_chain_handles = std::array{ sc.handle };
      auto presentation_info  = VkPresentInfoKHR{
         .sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .waitSemaphoreCount  = signal_semaphores.size(),
         .pWaitSemaphores     = signal_semaphores.data(),
         .swapchainCount      = swap_chain_handles.size(),
         .pSwapchains         = swap_chain_handles.data(),
         .pImageIndices       = &sc_image_index,
         .pResults            = nullptr,
      };
      result = vkQueuePresentKHR(this->queues.presentation.handle, &presentation_info);
      switch (result) {
         case VK_SUCCESS:
            if (this->widget.resized)
               this->handle_resize();
            break;
         case VK_ERROR_OUT_OF_DATE_KHR:
         case VK_SUBOPTIMAL_KHR:
            this->handle_resize();
            break;
         default:
            throw std::runtime_error("[DovahKitVulkanSubsystem][drawFrame] Failed to present swap chain image.");
      }
      //
      // Post-draw behavior:
      //
      this->_execute_pending_scene_deletions();
   }


   command_buffer surface_renderer::_begin_one_time_commands() {
      //
      // TODO: This is a useful helper function, but you'll actually get higher throughput if you 
      // reuse a single command buffer instead of spawning several temporary buffers; you'd want 
      // to have a function to create that single reusable buffer, and a "flush" function to 
      // execute whatever commands have been recorded so far.
      // 
      // See the end of: https://vulkan-tutorial.com/en/Texture_mapping/Images#page_Transition-barrier-masks
      //
      auto scratch = command_buffer(*this);
      //
      auto begin_info = VkCommandBufferBeginInfo{
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      };
      vkBeginCommandBuffer(scratch.handle, &begin_info);
      //
      return scratch;
   }
   void surface_renderer::_end_one_time_commands(command_buffer& scratch) {
      vkEndCommandBuffer(scratch.handle);
      //
      auto submit_info = VkSubmitInfo{
         .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1,
         .pCommandBuffers    = &scratch.handle,
      };
      vkQueueSubmit(this->queues.graphics.handle, 1, &submit_info, VK_NULL_HANDLE);
      vkQueueWaitIdle(this->queues.graphics.handle);
   }


   VkExtent2D surface_renderer::desired_surface_size() const {
      QWidget* w = this->widget.pointer;
      if (!w || !w->isVisible())
         return { 0, 0 };
      return { (uint32_t)w->width(), (uint32_t)w->height() };
   }
   VkFormat surface_renderer::find_depth_format() const {
      auto fmt = this->device_info->find_supported_format(
         { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
         VK_IMAGE_TILING_OPTIMAL,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
      if (fmt == VK_FORMAT_UNDEFINED) {
         throw std::runtime_error("[vulkanDK::surface_renderer::find_depth_format] No format.");
      }
      return fmt;
   }
   bool surface_renderer::needs_null_texture() const {
      return this->device_info->support.descriptor_bindings.null_handles == false;
   }

   buffer surface_renderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      buffer out = buffer(*this);
      //
      auto buffer_info = VkBufferCreateInfo{
         .sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size         = size,
         .usage        = usage,
         .sharingMode  = VK_SHARING_MODE_EXCLUSIVE,
      };
      if (vkCreateBuffer(this->logical_device, &buffer_info, nullptr, &out.handle) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::create_buffer] Failed to create vertex buffer.");
      }
      //
      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(this->logical_device, out.handle, &memRequirements);
      out.size = memRequirements.size;
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
         .memoryTypeIndex = this->device_info->find_memory_type(memRequirements.memoryTypeBits, properties),
      };
      if (vkAllocateMemory(this->logical_device, &alloc_info, nullptr, &out.memory) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::create_buffer] Failed to allocate vertex buffer memory.");
      }

      vkBindBufferMemory(this->logical_device, out.handle, out.memory, 0);

      return out;
   }

   #pragma region scene
   size_t surface_renderer::add_texture(const QString& texture_path) {
      constexpr size_t fail = std::string::npos;
      //
      auto& list = this->scene.textures;
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
            qDebug("[vulkanDK::surface_renderer::add_texture] Failed to open test image.");
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
         qDebug("[vulkanDK::surface_renderer::add_texture] Failed to load test image.");
         return fail;
      }
      uint32_t w = texture.width();
      uint32_t h = texture.height();
      //
      // here, we may want to lock the texture asset list, if we were doing a multithreaded renderer
      //
      auto texture_index = this->scene.insert_new_texture();
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
      auto  staging = this->create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data    = staging.map_memory();
      memcpy(data, texture.constBits(), image_size);
      staging.unmap_memory(data);
      //
      texture = QImage();
      //
      target.content = concrete_image(*this);
      target.content.create_image(
         w, h,
         VK_FORMAT_R8G8B8A8_SRGB,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      target.content.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      target.content.copy_content_from_buffer(staging.handle);
      target.content.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      target.content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      target.frame_dirty_flags.set_all();
      return texture_index;
   }
   void surface_renderer::add_mesh(const QString& texture_path) {
      size_t texture_index = this->add_texture(texture_path);
      if (texture_index == std::string::npos) {
         qDebug("Cannot add new rendered object: failed to add its texture.");
         return;
      }
      auto&  texture_item = this->scene.textures[texture_index];
      size_t object_index = this->scene.insert_new_mesh();
      if (object_index == std::string::npos) {
         qDebug("Cannot add new rendered objects. Maximum has been reached.");
         if (texture_item.refcount == 0) {
            //
            // This texture was created for us, but we never got a chance to use it. Mark it 
            // for deletion.
            //
            texture_item.pending_delete = true;
            texture_item.frame_dirty_flags.set_all();
         }
         return;
      }
      QSize texture_size = { (int)texture_item.w, (int)texture_item.h }; // just used to size the quad so we maintain aspect ratio
      {  // Create model
         if (!texture_size.isValid())
            texture_size = { 1, 1 };
         //
         auto& ro  = this->scene.meshes[object_index];
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
         auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         void* data    = staging.map_memory();
         memcpy((void*)((std::intptr_t)data),                 vertices.data(), buffer_size_v);
         memcpy((void*)((std::intptr_t)data + buffer_size_v), indices.data(),  buffer_size_i);
         staging.unmap_memory(data);
         //
         vib.indices_at  = buffer_size_v;
         vib.index_count = indices.size();
         vib.buffer      = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         vib.buffer.copy_from(staging);
         //
         ro.shader_params.transform = transform;
         ro.frame_dirty_flags.set_all();
      }
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }
   void surface_renderer::remove_mesh(size_t i) {
      auto& list = this->scene.meshes;
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
      item.pending_delete = true;
      item.frame_dirty_flags.set_all();
      {
         auto ti = item.texture_index;
         if (ti >= 0) {
            auto& list = this->scene.textures;
            if (ti < list.size()) {
               auto& tex = list[ti];
               if (--tex.refcount == 0) {
                  tex.pending_delete = true;
                  tex.frame_dirty_flags.set_all();
               }
            }
         }
      }
      item.texture_index = -1;
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }
   void surface_renderer::remove_last_mesh() {
      auto& list = this->scene.meshes;
      auto  size = list.size();
      if (size == 0)
         return;
      for (size_t i = size - 1; i >= 0; --i) {
         auto& item = list[i];
         if (item.empty() || item.pending_delete)
            continue;
         this->remove_mesh(i);
         return;
      }
   }

   void surface_renderer::_execute_pending_scene_deletions() {
      auto& pd = this->scene.pending_deletions;
      if (pd.meshes) {
         size_t deleted    = 0;
         size_t last_alive = 0;
         auto&  list       = this->scene.meshes;
         for (auto& item : list) {
            if (item.pending_delete && !item.frame_dirty_flags.any_set()) {
               item.reset();
               ++deleted;
            } else {
               ++last_alive;
            }
         }
         pd.meshes -= deleted;
         list.resize(last_alive + 1);
      }
      if (pd.textures) {
         size_t deleted    = 0;
         size_t last_alive = 0;
         auto&  list       = this->scene.textures;
         for (auto& item : list) {
            if (item.pending_delete && !item.frame_dirty_flags.any_set()) {
               item.reset();
               ++deleted;
            } else {
               ++last_alive;
            }
         }
         pd.textures -= deleted;
         list.resize(last_alive + 1);
      }
   }
   #pragma endregion
}