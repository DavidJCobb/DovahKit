#include "surface_renderer.h"
#include <QResource> // for loading shaders
#include "frame_in_flight.h"
#include "logical_device.h"
#include "material.h"
#include "physical_device.h"
#include "queue_family_info.h"
#include "render_pass.h"
#include "shader_module.h"
#include "surface.h"
#include "vertex.h"
#include "config/frames_in_flight.h"
#include "config/scene_limits.h"

// loading textures from files using Qt:
#include <QBuffer>
#include <QImage>
#include <QImageReader>

#include "loaded_texture.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
   constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;
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
   surface_renderer::surface_renderer(surface& s, logical_device& d) : target(s), device(d), swap_chain(*this), null_texture(*this) {
      this->descriptor_set_definition.bindings = {
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
      this->device.on_dependent_object_created(*this);
      this->setup();
   }
   surface_renderer::~surface_renderer() {
      this->teardown();
      this->device.on_dependent_object_deleted(*this);
   }

   void surface_renderer::setup() {
      {
         this->descriptor_set_definition.set_device(this->device.handle);
         this->descriptor_set_definition.setup();
      }
      this->_setup_shader_modules();
      this->_setup_texture_sampler(); // descriptor set layout must be able to refer to our immutable sampler
      //
      this->_setup_command_pool(); // Cannot copy buffers, etc., for texture loading, scene setup, etc., until the command pool is ready.
      this->_setup_descriptor_pool();
      //
      this->swap_chain.setup(); // includes frames-in-flight, which in turn includes semaphores, shader parameter buffers, descriptor set allocations (though not their contained descriptors), and command buffers
      this->_setup_render_passes();
      this->_create_null_texture();
      this->_setup_initial_scene();
      this->_initialize_descriptor_sets();
      //
      this->target._on_renderer_ready();
   }
   void surface_renderer::_setup_shader_modules() {
      auto* dfn = new material_definition;
      this->swap_chain.material_definitions.push_back(dfn);
      //
      shader_module* frag = nullptr;
      shader_module* vert = nullptr;
      {
         frag = new shader_module(this->device, QResource("shaders/shader.frag.spv").uncompressedData());
         vert = new shader_module(this->device, QResource("shaders/shader.vert.spv").uncompressedData());
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
      dfn->stages = {
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
   }
   void surface_renderer::_setup_texture_sampler() {
      const auto& physical = this->device.physical;
      const auto& support  = physical.support;
      //
      auto sampler_info = VkSamplerCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter        = VK_FILTER_LINEAR,
         .minFilter        = VK_FILTER_LINEAR,
         .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
         .addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .mipLodBias       = 0.0,
         .anisotropyEnable = support.max_anisotropic_filtering > 0.0 ? VK_TRUE : VK_FALSE,
         .maxAnisotropy    = std::min(8.0F, support.max_anisotropic_filtering),
         .compareEnable    = VK_FALSE,
         .compareOp        = VK_COMPARE_OP_ALWAYS,
         .minLod           = 0.0,
         .maxLod           = 0.0,
         .borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
         .unnormalizedCoordinates = VK_FALSE, // true: coordinates are [0, width], etc; false: coordinates are [0, 1]
      };
      if (vkCreateSampler(this->device.handle, &sampler_info, nullptr, &this->texture_sampler) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_texture_sampler] Failed to create the texture sampler.");
      }
   }
   //
   void surface_renderer::_setup_command_pool() {
      auto indices   = queue_family_info(this->device.physical, this->target);
      auto pool_info = VkCommandPoolCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         .queueFamilyIndex = indices.families.graphics,
      };
      if (vkCreateCommandPool(this->device.handle, &pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_command_pool] Failed to create the command pool.");
      }
   }
   void surface_renderer::_setup_descriptor_pool() {
      auto& dl = this->descriptor_set_definition;
      //
      auto image_count = config::frames_in_flight_count;
      //
      std::vector<VkDescriptorPoolSize> sizes;
      for (auto& binding : dl.bindings) {
         auto count = binding.count;
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
      if (vkCreateDescriptorPool(this->device.handle, &pool_info, nullptr, &this->descriptor_pool) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_descriptor_pool] Failed to create the descriptor pool.");
      }
   }
   void surface_renderer::_create_null_texture() {
      if (this->device.physical.support.descriptor_bindings.null_handles)
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
         auto staging = this->device.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
      auto device = this->device.handle;
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
            auto staging = this->device.create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
            auto staging = this->device.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            //
            void* data = staging.map_memory();
            memcpy((void*)((std::intptr_t)data),                 s.vertices.data(), buffer_size_v);
            memcpy((void*)((std::intptr_t)data + buffer_size_v), s.indices.data(),  buffer_size_i);
            staging.unmap_memory(data);
            //
            vib.indices_at     = buffer_size_v;
            vib.index_count    = s.indices.size();
            vib.allocated_size = buffer_size;
            //
            vib.buffer = this->device.create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vib.buffer.copy_from(*this, staging);
            d.shader_params.transform = s.transform;
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
         vkUpdateDescriptorSets(this->device.handle, (uint32_t)descriptor_writes.size(), descriptor_writes.data(), 0, nullptr);
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
         rp->attachments = {
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
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after drawing, so let the driver decide how best to discard it
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
                  .color = {
                     VkAttachmentReference{ // color
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // depth
                     .attachment = 1,
                     .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpasses.dependencies = {
            VkSubpassDependency{
               .srcSubpass    = VK_SUBPASS_EXTERNAL,
               .dstSubpass    = 0,
               .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask = 0,
               .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            }
         };
      }
      //
      // (Re)create the render passes within the GPU:
      //
      for(auto* rp : this->render_passes)
         rp->setup();
   }

   void surface_renderer::teardown() {
      this->target._on_renderer_teardown_imminent();
      //
      vkDeviceWaitIdle(this->device.handle); // wait for all draw commands to finish (remember: they're asynch)
      //
      // Ensure all child objects belonging to the instance are destroyed.
      //
      this->scene.teardown();
      this->null_texture.teardown();
      {
         auto& list = this->render_passes;
         for (auto* rp : this->render_passes)
            delete rp;
         list.clear();
      }
      this->swap_chain.teardown();
      vkDestroyDescriptorPool(this->device.handle, this->descriptor_pool, nullptr);
      vkDestroyCommandPool(this->device.handle, this->command_pool, nullptr);
      //
      vkDestroySampler(this->device.handle, this->texture_sampler, nullptr);
      {
         auto& list = this->shader_modules;
         for (auto* sm : list)
            delete sm;
         list.clear();
      }
      this->descriptor_set_definition.teardown();
      //
      this->target._on_renderer_teardown_complete();
   }

   void surface_renderer::handle_resize() {
      auto  device = this->device.handle;
      auto& sc     = this->swap_chain;
      //
      VkFormat sc_format = sc.format;
      {  // Tear down swap chain state
         for (auto& fif : sc.frames_in_flight) {
            fif.invalidate_all_command_buffers(); // FIF doesn't have any other state that we'd need to reset
         }
         //vkDestroyDescriptorPool(device, this->descriptor_pool, nullptr); // only necessary if the frame-in-flight count has changed
         /*// We don't actually need to tear down the render passes unless the info they needed from the swap chain (i.e. the swap chain image format) has changed.
         for (auto* rp : this->render_passes) {
            rp->teardown();
         }
         //*/
         sc.teardown();
      }
      {  // Set up new state
         sc.setup();
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
         // Rebuilding the descriptor pool is only necessary if the frame-in-flight count has changed.
         this->_initialize_descriptor_sets();
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
      this->target.state.resized = false;
   }


   void surface_renderer::draw_next_frame() {
      this->scene.update();
      //
      constexpr auto no_timeout = UINT64_MAX;
      //
      //  - Acquire an image from the swap chain
      //  - Execute the command buffer with that image as attachment in the framebuffer
      //  - Return the image to the swap chain for presentation
      // 
      // These tasks are asynchronous, but must run sequentially.
      //
      if (!this->target.state.visible)
         return;
      auto pf = this->swap_chain.advance_frame();
      switch (pf.result) {
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
      this->swap_chain.confirm_frame(pf);
      switch (pf.result) {
         case VK_SUCCESS:
            if (this->target.state.resized)
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
      //
      //
      static_assert(false, "TODO: scene post-frame behavior (delete detached objects, etc.)");
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
      vkQueueSubmit(this->device.queues.graphics, 1, &submit_info, VK_NULL_HANDLE);
      vkQueueWaitIdle(this->device.queues.graphics);
   }


   VkFormat surface_renderer::find_depth_format() const {
      auto fmt = this->device.physical.find_supported_format(
         { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
         VK_IMAGE_TILING_OPTIMAL,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
      );
      if (fmt == VK_FORMAT_UNDEFINED) {
         throw std::runtime_error("[vulkanDK::surface_renderer::find_depth_format] No format.");
      }
      return fmt;
   }

   #pragma region scene
   size_t surface_renderer::add_texture(const QString& texture_path);
   void surface_renderer::add_mesh(const QString& texture_path);
   void surface_renderer::remove_mesh(size_t i);
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
   #pragma endregion
}