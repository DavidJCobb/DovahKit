#include "surface_renderer.h"
#include <chrono>
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
#include "config/use_inverted_depth.h"
#include "config/validation_layers.h"

// loading textures from files using Qt:
#include <QBuffer>
#include <QImage>
#include <QImageReader>

#include "loaded_texture.h"
#include "rendered_mesh.h"
#include "scene_global_state.h"
#include "dds/texture.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"
//
#include <QDir>
#include <QFile>
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// loading NIFs
#include "nif/file.h"
#include "nif/blocks/BSEffectShaderProperty.h"
#include "nif/blocks/BSLightingShaderProperty.h"
#include "nif/blocks/BSShaderTextureSet.h"
#include "nif/blocks/BSTriShape.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiTriShape.h"
#include "nif/blocks/NiTriShapeData.h"

namespace {
   static constexpr bool debug_log_scene_object_lifetimes = false;
}

namespace {
   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
   };

   static constexpr auto desired_swap_chain_presentation_mode  = VK_PRESENT_MODE_MAILBOX_KHR;
   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;
}

#include "overlays/fps.h"
namespace {
   static constexpr bool setup_fps_counter = true; // mainly just used for grouping code, tbh
}

namespace {
   constexpr std::array<vulkanDK::vertex, 4> _make_quad(float hfwc, bool test_colors) { // height-for-width, centered
      using namespace vulkanDK;
      //
      std::array<vulkanDK::vertex, 4> out = {};
      for (size_t i = 0; i < out.size(); ++i) {
         auto& v = out[i];
         v.color = { 1, 1, 1 };
         v.uv.x = (i % 3 == 0) ? 1 : 0;
         v.uv.y = (i > 1)      ? 1 : 0;
         v.normal    = { 0, 0, 1 };
         v.tangent   = { 1, 0, 0 };
         v.bitangent = { 0, 1, 0 };
         //
         v.pos.x = (i % 3) ? 0.5 : -0.5;
         v.pos.y = (i > 1) ? hfwc : -hfwc;
         v.pos.z = 0;
      }
      if (test_colors) {
         for (size_t i = 0; i < out.size(); ++i) {
            auto& v = out[i];
            v.color.r = (i == 0 || i == 3) ? 1 : 0;
            v.color.g = (i == 1 || i == 3) ? 1 : 0;
            v.color.b = (i == 2 || i == 3) ? 1 : 0;
         }
      }
      //
      return out;
   }
   constexpr std::vector<vulkanDK::vertex> _make_quad(float hfwc) {
      std::vector<vulkanDK::vertex> items(4);
      const auto& arr = _make_quad(hfwc, true);
      for (size_t i = 0; i < 4; ++i)
         items[i] = arr[i];
      return items;
   }
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
         _make_quad(0.395),
         { 0, 1, 2, 2, 3, 0 },
         glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            { 0.0, 0.0, 0.7 }
         ),
      },
      _model{  // screenshot of Tolfdir
         _make_quad(0.28125),
         { 0, 1, 2, 2, 3, 0 },
         glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            { 0.0, 0.0, -0.5 }
         ),
      },
      _model{  // screenshot of books
         _make_quad(0.28125),
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
         vulkanDK::descriptor_binding{ // uniform buffer object: vulkanDK::scene_global_state
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
      if constexpr (setup_fps_counter) {
         assert(this->descriptor_set_layouts.size() == 1);
         this->descriptor_set_layouts.emplace_back().bindings = { // FPS counter
            vulkanDK::descriptor_binding{ // uniform buffer object
               .index              = 0,
               .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               .count              = 1,
               .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
               .immutable_samplers = nullptr,
            },
            vulkanDK::descriptor_binding{ // texture sampler
               .index              = 1,
               .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               .count              = 1,
               .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
               .immutable_samplers = nullptr,
            },
         };
      }
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

   void surface_renderer::_setup_raw_pixel_texture_sampler() {
      //
      // Shaders wishing to use this texture sampler must be aware:
      // 
      //  - coordinates are relative to the texture size (i.e. [0, w], not [0, 1]); this is the meaning 
      //    of the "unnormalized coordinates" option, and will help avoid artifacts
      // 
      //     - this only affects UVs. if you want to display a mesh using screen coordinates (e.g. for 
      //       UI that remains at a fixed size rather than scaling with the viewport), then your shader 
      //       will need to be given the viewport size as an input and will need to manually convert 
      //       vertex positions from screen space to clip space (that is, [-1, 1]). that conversion is 
      //       done as: (2 * (screen_coord / screen_dimension) - 1.0).
      // 
      //  - use GLSL textureLod() to pull color/pixel values given a texture and UVs. the GLSL texture() 
      //    function is not compatible with unnormalized coordinates
      //
      auto sampler_info = VkSamplerCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter        = VK_FILTER_NEAREST,
         .minFilter        = VK_FILTER_NEAREST, // both filters must be the same for unnormalized coords
         .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_NEAREST, // must be nearest for unnormalized coords, but we want nearest anyway
         .addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT,
         .mipLodBias       = 0.0,
         .anisotropyEnable = VK_FALSE, // must be false for unnormalized coords, but we want false anyway
         .maxAnisotropy    = 0.0F,
         .compareEnable    = VK_FALSE, // must be false for unnormalized coords, but we want false anyway
         .compareOp        = VK_COMPARE_OP_ALWAYS,
         .minLod           = 0.0, // must be zero for unnormalized coords
         .maxLod           = 0.0, // must be zero for unnormalized coords
         .borderColor      = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
         .unnormalizedCoordinates = VK_TRUE, // true: coordinates are [0, width], etc; false: coordinates are [0, 1]
      };
      if (vkCreateSampler(this->logical_device, &sampler_info, nullptr, &this->raw_pixel_texture_sampler) != VK_SUCCESS) {
         throw std::runtime_error("[vulkanDK::surface_renderer::_setup_raw_pixel_texture_sampler] Failed to create the raw pixel texture sampler.");
      }
   }
   void surface_renderer::_teardown_raw_pixel_texture_sampler() {
      if (this->raw_pixel_texture_sampler != VK_NULL_HANDLE) {
         vkDestroySampler(this->logical_device, this->raw_pixel_texture_sampler, nullptr);
         this->raw_pixel_texture_sampler = VK_NULL_HANDLE;
      }
   }

   void surface_renderer::setup() {
      if (this->logical_device == VK_NULL_HANDLE) {
         return;
      }
      if constexpr (use_vma_library) {
         VmaAllocatorCreateInfo allocatorInfo = {};
         allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
         allocatorInfo.physicalDevice   = this->device_info->handle;
         allocatorInfo.device           = this->logical_device;
         allocatorInfo.instance         = this->owner.getHandle();
         //
         vmaCreateAllocator(&allocatorInfo, &this->allocator);
      }
      this->swap_chain.frames_in_flight.resize(config::frames_in_flight_count);
      //
      this->setup_descriptor_set_layouts();
      this->_define_render_passes();
      this->_setup_shaders();
      this->setup_texture_sampler(); // descriptor set layout must be able to refer to our immutable sampler
      this->_setup_raw_pixel_texture_sampler();
      //
      this->setup_command_pool(this->queues.graphics.index);
      //
      {  // swap chain
         this->_setup_swap_chain_instance();         // sets up format, extent size, and handle
         this->_setup_render_passes();               // requires swap chain format
         for (auto* s : this->shaders)
            s->setup_pipeline(this->surface_extent);
         this->_setup_depth_buffer();                // requires extent size
         this->_setup_swap_chain_images();
         this->_setup_framebuffers();                // requires swap chain image count and view handles
         this->setup_descriptor_pool();              // requires swap chain image count
         this->_setup_swap_chain_image_frame_data(); // requires descriptor pool
         for (auto& fif : this->swap_chain.frames_in_flight)
            fif.setup(*this);
      }
      this->_create_null_texture();
      this->scene.update_projection(this->surface_extent); // requires extent size
      this->_setup_initial_scene();
      this->_initialize_descriptor_sets();
      //
      this->_on_renderer_ready();
   }
   void surface_renderer::_define_render_passes() {
      {
         auto* rp = this->render_passes_by_name.main = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
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
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses.descriptions = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = { // there can be multiple color attachments
                     VkAttachmentReference{
                        .attachment = 0,
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
            // For a subpass  dependency, a "task" is a  read or write  operation (access mask) 
            // and the stages in which that operation occurs (stage mask). Because you can list 
            // multiple stages, many accesses are defined on a per-stage basis (e.g. a specific 
            // flag for "reading the color attachment,"  rather than a single flag for "read").
            // 
            // The start of a subpass  has an implicit  task: transitioning  the target image's 
            // current layout  to the one specified by the relevant attachment's  initialLayout 
            // field above. We of course need to ensure that the image in question (typically a 
            // swap chain image) is actually available (i.e. has been acquired) before any such 
            // transition is attempted.
            // 
            // If you don't specify a first external dependency  -- that is, a dependency whose 
            // source is  VK_SUBPASS_EXTERNAL -- then Vulkan  will inject a default  with these 
            // settings:
            // 
            //    .srcSubpass      = VK_SUBPASS_EXTERNAL,
            //    .dstSubpass      = /* first subpass the attachment is used in */,
            //    .srcStageMask    = VK_PIPELINE_STAGE_NONE_KHR,
            //    .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            //    .srcAccessMask   = VK_ACCESS_NONE_KHR, // means all accesses; same as 0
            //    .dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            //    .dependencyFlags = 0,
            //
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            //
            // If you don't specify a final external dependency  -- that is, a dependency whose 
            // destination  is VK_SUBPASS_EXTERNAL  -- then  Vulkan will inject one  with these 
            // settings:
            // 
            //    .srcSubpass      = /* based on the last subpass */,
            //    .dstSubpass      = VK_SUBPASS_EXTERNAL,
            //    .srcStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            //    .dstStageMask    = VK_PIPELINE_STAGE_NONE_KHR,
            //    .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            //    .dstAccessMask   = VK_ACCESS_NONE_KHR, // means all accesses; same as 0
            //    .dependencyFlags = 0,
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
      {
         auto* rp = this->render_passes_by_name.ui = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
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
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpasses.dependencies = {
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
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
      this->render_passes = { this->render_passes_by_name.main, this->render_passes_by_name.ui };
   }
   void surface_renderer::_setup_shaders() {
      {  // Material: "MainMatl"
         auto* s = this->get_or_create_shader(main_shader_id);
         s->set_render_pass(this->render_passes_by_name.main);
         s->set_layout_info(
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
         //
         auto& dfn = s->definition;
         //
         shader_module* frag = nullptr;
         shader_module* vert = nullptr;
         {
            frag = new shader_module(this->logical_device, QResource("shaders/shader.frag.spv").uncompressedData());
            vert = new shader_module(this->logical_device, QResource("shaders/shader.vert.spv").uncompressedData());
            if (frag->empty()) {
               throw std::runtime_error("[vulkanDK::surface_renderer::_setup_shaders] Failed to load fragment shader.");
            }
            if (vert->empty()) {
               throw std::runtime_error("[vulkanDK::surface_renderer::_setup_shaders] Failed to load vertex shader.");
            }
            this->shader_modules.push_back(frag);
            this->shader_modules.push_back(vert);
         }
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
         if constexpr (config::use_inverted_depth) {
            dfn.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         {
            auto& vertex     = dfn.inputs.vertex;
            auto  attributes = vertex::getAttributeDescriptions();
            vertex.bindings.push_back(vertex::getBindingDescription());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         //
         // And be sure to set up the pipeline layout when you're done!
         //
         s->setup_pipeline_layout(*this);
      }
      //
      // FPS counter:
      //
      if constexpr (setup_fps_counter) {
         vulkanDK::overlays::fps::setup_shaders(*this);
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
         {
            .extent = {
               .width  = w,
               .height = h,
            },
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         },
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
               throw std::runtime_error("[vulkanDK::surface_renderer::_setup_initial_scene] Failed to load test image.");
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
               {
                  .extent = {
                     .width  = w,
                     .height = h,
                  },
                  .format = VK_FORMAT_R8G8B8A8_SRGB,
                  .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               },
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
            //
            target.life_state = scene_frame_item_state::active;
            target.handled_frames.set_all_out_of_date();
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
            d.texture_indices.diffuse = i; // TODO: in the future we'd load objects and textures together, basically; for our simple test, the default 3 objects and their textures load separately
            {
               ++this->scene.textures[d.texture_indices.diffuse].refcount;
            }
            {
               d.anim_state = new mesh_animation_state;
            }
            //
            d.data.vertices = s.vertices;
            d.data.indices  = s.indices;
            d.recalc_bounding_sphere();
            //
            VkDeviceSize buffer_size_v;
            VkDeviceSize buffer_size_i;
            VkDeviceSize buffer_size;
            d.sizes_for_setup(buffer_size_v, buffer_size_i, buffer_size);
            //
            auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data    = staging.map_memory();
            d.setup_vib_data_at(data);
            staging.unmap_memory(data);
            //
            vib.wide_indices = d.data.indices.type() == vertex_index_list::value_type::wide;
            vib.indices_at   = buffer_size_v;
            vib.index_count  = s.indices.size();
            vib.buffer       = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vib.buffer.copy_from(staging);
            //
            d.shader_params.transform = s.transform;
            d.life_state = scene_frame_item_state::active;
            d.handled_frames.set_all_out_of_date();
         }
      }
      //
      // Scene sky and floor:
      //
      {
         auto ti = this->add_texture(":/shaders/white.png");
         { // Mesh: floor
            auto  mi   = this->scene.insert_new_mesh();
            auto& mesh = this->scene.meshes[mi];
            auto& vib = mesh.vertex_and_index_buffer;
            mesh.texture_indices.diffuse = ti;
            {
               ++this->scene.textures[ti].refcount;
            }
            //
            {
               constexpr float size = 99999;
               mesh.data.vertices = {  // Vertices
                  {
                     .pos    = { -size, -size, 0 },
                     .color  = { 1.0, 0.6, 0.0 },
                     .uv     = { 1, 0 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { size, -size, 0 },
                     .color  = { 1.0, 0.6, 0.0 },
                     .uv     = { 0, 0 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { size, size, 0 },
                     .color  = { 1.0, 0.6, 0.0 },
                     .uv     = { 0, 1 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { -size, size, 0 },
                     .color  = { 1.0, 0.6, 0.0 },
                     .uv     = { 1, 1 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
               };
            }
            mesh.data.indices  = { 0, 1, 2, 2, 3, 0 };
            mesh.recalc_bounding_sphere();
            //
            VkDeviceSize buffer_size_v;
            VkDeviceSize buffer_size_i;
            VkDeviceSize buffer_size;
            mesh.sizes_for_setup(buffer_size_v, buffer_size_i, buffer_size);
            //
            auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data    = staging.map_memory();
            mesh.setup_vib_data_at(data);
            staging.unmap_memory(data);
            //
            vib.wide_indices = mesh.data.indices.type() == vertex_index_list::value_type::wide;
            vib.indices_at   = buffer_size_v;
            vib.index_count  = mesh.data.indices.size();
            vib.buffer       = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vib.buffer.copy_from(staging);
            //
            mesh.shader_params.transform = glm::mat4(1);
            mesh.life_state = scene_frame_item_state::active;
            mesh.handled_frames.set_all_out_of_date();
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
      auto& sc = this->swap_chain;
      for (size_t i = 0; i < sc.images.size(); ++i) {
         auto& frame = sc.images[i];
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
         //
         if constexpr (setup_fps_counter) {
            frame.overlays.fps.initialize_descriptor_sets(*this, frame);
         }
      }
      //
      // Mark textures as synchronized:
      //
      for (auto& entry : this->scene.textures)
         entry.handled_frames.set_all_up_to_date(sc.images.size());
   }
   //
   void surface_renderer::_setup_render_passes() {
      this->render_passes_by_name.main->attachments[0].format = this->swap_chain.format;
      this->render_passes_by_name.ui->attachments[0].format = this->swap_chain.format;
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
   void surface_renderer::_setup_depth_buffer() {
      auto  extent = this->surface_extent;
      auto  format = this->find_depth_format();
      auto& db     = swap_chain.depth_buffer;
      db = concrete_image(*this);
      db.create_image(
         {
            .extent = {
               .width  = extent.width,
               .height = extent.height,
            },
            .format = format,
            .usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
         }, 
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
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
         sc.images.resize(image_count);
      }
      this->configuration.image_count = image_count; // for superclass stuff including descriptor pool sizing
      //
      // Get the image handles and set up image state (descriptor sets, views, etc.):
      //
      std::vector<VkImage> image_handles(image_count);
      vkGetSwapchainImagesKHR(this->logical_device, sc.handle, &image_count, image_handles.data());
      for (size_t i = 0; i < image_count; ++i) {
         sc.images[i].setup(*this, i);
         //
         sc.images[i].image = surface_renderer_image_view(*this, image_handles[i]);
         sc.images[i].image.create_basic_view(sc.format, VK_IMAGE_ASPECT_COLOR_BIT);
      }
   }
   void surface_renderer::_setup_swap_chain_image_frame_data() {
      for (auto& image : this->swap_chain.images)
         image.setup_descriptor_sets();
   }
   void surface_renderer::_setup_framebuffers() {
      auto& sc = this->swap_chain;
      auto extent = this->surface_extent;
      auto r_pass = this->render_passes[0]->handle;
      //
      for (auto& image : sc.images) {
         auto attachments = std::array{ image.image.view, sc.depth_buffer.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = r_pass,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1,
         };
         if (vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &image.framebuffer) != VK_SUCCESS) {
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
      {
         auto& list = this->shaders;
         for (auto* e : list)
            delete e;
         list.clear();
      }
      {  // Swap chain
         auto& sc = this->swap_chain;
         //
         sc.images.clear();
         sc.depth_buffer.teardown();
         vkDestroySwapchainKHR(this->logical_device, sc.handle, nullptr);
         sc.handle = VK_NULL_HANDLE;
         //
         sc.frames_in_flight.clear();
      }
      this->_teardown_raw_pixel_texture_sampler();
      for (auto& e : this->render_passes_by_name._list)
         e = nullptr; // deletion will be handled in abstract_renderer::teardown
      //
      if constexpr (use_vma_library) {
         vmaDestroyAllocator(this->allocator);
      }
      //
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
      size_t   sc_count  = sc.images.size();
      {  // Tear down swap chain state
         for (auto& s : this->shaders)
            s->pre_resize();
         for (auto& image : sc.images) {
            image.teardown_descriptor_sets(); // TODO: we don't actually have to free and rebuild these if the descriptor pool itself doesn't need to be rebuilt
            image.teardown();
         }
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
            if (auto* rp = this->render_passes_by_name.main) {
               rp->teardown();
               rp->attachments[0].format = sc.format;
               rp->setup();
            }
            if (auto* rp = this->render_passes_by_name.ui) {
               rp->teardown();
               rp->attachments[0].format = sc.format;
               rp->setup();
            }
         }
         for (auto& s : this->shaders)
            s->post_resize(this->surface_extent);
         this->_setup_depth_buffer(); // requires surface extent
         this->_setup_swap_chain_images();
         if (sc.images.size() != sc_count) {
            //
            // If the swap chain image count has changed, then we must ensure that the descriptor 
            // pool is large enough to hold descriptor sets and descriptors for each image.
            //
            this->teardown_descriptor_pool();
            this->setup_descriptor_pool();
         }
         this->_setup_framebuffers(); // requires surface extent and image view handle
         this->_setup_swap_chain_image_frame_data(); // requires descriptor pool
         this->_initialize_descriptor_sets();
         //
         sc.current_frame = 0;
      }
      this->scene.update_projection(this->surface_extent);
      //
      // The above procedure will have reset all shader-side data for rendered objects, 
      // so we need to mark all rendered objects as dirty so we resynchronize that. We 
      // don't have to update descriptors the same way because we just took care of them 
      // when initializing descriptor sets -- that is, textures are already dealt with.
      //
      for (auto& mesh : this->scene.meshes)
         mesh.handled_frames.set_all_out_of_date();
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
      // We need to perform three operations:
      // 
      //  - ACQUIRE an image from the swap chain.
      // 
      //  - SUBMIT command buffers to draw to that swap chain image and its framebuffer.
      // 
      //  - PRESENT the swap chain image, so that it can display on the monitor.
      // 
      // These operations must occur in order,  but the API calls are asynchronous, so we 
      // must manually synchronize them. We can rely on semaphores for this.
      // 
      // A "fence" allows  the CPU to synchronize with (i.e. wait for)  some task running 
      // on the GPU,  whereas a "semaphore" allows one task on the GPU to  wait for other 
      // tasks on the GPU.
      //
      if (!this->widget.visible)
         return;
      //
      using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
      auto time_prior = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      //
      // If this frame-in-flight is still being used to render and present another swap 
      // chain image, wait for it to finish. We'll also advance the current frame counter 
      // here.
      //
      auto& fif = sc.frames_in_flight[sc.current_frame];
      sc.current_frame = (sc.current_frame + 1) % sc.frames_in_flight.size();
      vkWaitForFences(this->logical_device, 1, &fif.fence, VK_TRUE, no_timeout);
      //
      // Next, let's acquire a swap chain image to use for this frame.
      // 
      // By default, vkAcquireNextImageKHR is allowed to return a swap chain image before 
      // the presentation engine has actually finished reading from that image; we're the 
      // ones responsible for making sure that we don't write to the image until present-
      // ation has  actually finished. This,  by the way, implies that the  function also 
      // doesn't have to wait  for the GPU to finish  playing  whatever command buffer(s) 
      // are being used for the swap chain image.
      // 
      // Whatever combination  of semaphores and fences we use, then,  must be sufficient 
      // to ensure  that the image has finished  being presented and any command  buffers 
      // that were previously drawing to it have finished playing.
      // 
      // vkAcquireNextImageKHR will allow us to  provide it with a semaphore handle and a 
      // fence handle.  Any provided handles  will be signalled later on,  when the image 
      // has been presented.
      // 
      // Apparently, semaphores are *unsignalled* if:
      // 
      //  - The batch of work that originally signalled the semaphore is finished.
      // 
      //  - A new batch of work which waits on the semaphore starts.
      // 
      // In this case, we're going to use the current frame's "image available" semaphore 
      // for acquisition: that semaphore will be  signalled when the newly-acquired image 
      // has been presented and can safely be drawn to.
      //
      uint32_t sc_image_index;
      auto     result = vkAcquireNextImageKHR(this->logical_device, sc.handle, no_timeout, fif.semaphores.image_available, VK_NULL_HANDLE, &sc_image_index);
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
      auto& sci = sc.images[sc_image_index];
      //
      // The acquired  image is initially in the VK_IMAGE_LAYOUT_UNDEFINED  layout, which 
      // we can't really use.  We have to transition it to a usable  layout before we can 
      // attempt to draw  to it. Our render pass is configured to do this  automatically, 
      // so the transition will happen when a command buffer begins the render pass.
      //
      // Let's hook the swap  chain image to the new frame-in-flight,  and then configure 
      // and submit its command buffers to render the image:
      //
      sci.draw(fif);
      //
      // Present the image.
      // 
      // Here, we  use the frame's "render finished"  semaphore as a  "wait semaphore." 
      // This means that the presentation operation will not begin until that semaphore 
      // is signalled. It will be signalled after the frame's "submit" operation (which 
      // executes command buffers) is complete.
      //
      auto wait_semaphores    = std::array{ fif.semaphores.render_finished };
      auto swap_chain_handles = std::array{ sc.handle };
      auto presentation_info  = VkPresentInfoKHR{
         .sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .pNext               = nullptr,
         .waitSemaphoreCount  = wait_semaphores.size(),
         .pWaitSemaphores     = wait_semaphores.data(),
         .swapchainCount      = swap_chain_handles.size(),
         .pSwapchains         = swap_chain_handles.data(),
         .pImageIndices       = &sc_image_index, // should be an array, one per swap chain handle; if just one handle, you can use a pointer to a single index
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
            throw std::runtime_error("[vulkanDK::surface_renderer::draw_next_frame] Failed to present swap chain image.");
      }
      //
      // Post-draw behavior:
      //
      auto time_after = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      {
         this->state.last_frame_time = std::chrono::duration<double, std::chrono::seconds::period>(time_after - time_prior).count();
      }
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
      auto scratch = command_buffer::create_transient(*this);
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
      return buffer::create(*this, size, usage, properties);
   }

   shader* surface_renderer::get_shader(cobb::eight_cc id) const {
      for (auto* s : this->shaders)
         if (s->id == id)
            return s;
      return nullptr;
   }
   shader* surface_renderer::get_or_create_shader(cobb::eight_cc id) {
      if (auto* s = this->get_shader(id))
         return s;
      auto* s = new shader;
      s->id = id;
      s->material.owner = this;
      this->shaders.push_back(s);
      return s;
   }

   #pragma region scene
   size_t surface_renderer::add_texture(const QString& texture_path) {
      constexpr size_t fail = std::string::npos;
      //
      auto& list = this->scene.textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         if (list[i].path == texture_path) {
            if constexpr (debug_log_scene_object_lifetimes) {
               qDebug("[vulkanDK::scene_renderer::add_texture] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(texture_path));
            }
            return i;
         }
      }
      //
      if (texture_path.endsWith(".dds", Qt::CaseInsensitive)) {
         auto file = QFile(texture_path);
         if (!file.open(QIODevice::ReadOnly)) {
            qDebug("[vulkanDK::surface_renderer::add_texture] Failed to open test image.");
            return fail;
         }
         auto bytearray = file.readAll();
         //
         dds::texture tex;
         tex.data = bytearray.constData();
         tex.size = bytearray.size();
         //
         if (!tex.read()) {
            qDebug("[vulkanDK::surface_renderer::add_texture] Failed to read DDS header.");
            return fail;
         }
         if (!tex.pixel_data() || !tex.pixel_data_size()) {
            qDebug("[vulkanDK::surface_renderer::add_texture] No DDS data available.");
            return fail;
         }
         //
         // Create scene texture.
         //
         auto texture_index = this->scene.insert_new_texture();
         if (texture_index == std::string::npos) {
            qDebug("[vulkanDK::scene_renderer::add_texture] Cannot add new rendered textures. Maximum has been reached.");
            return fail;
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            qDebug("[vulkanDK::scene_renderer::add_texture] Creating new texture at index %u for texture path <%s>", texture_index, qUtf8Printable(texture_path));
         }
         auto& target = list[texture_index];
         target.life_state = scene_frame_item_state::active;
         target.w    = tex.metadata.width;
         target.h    = tex.metadata.height;
         target.path = texture_path;
         //
         // Create Vulkan data:
         //
         target.content = concrete_image(*this);
         try {
            auto& img = target.content;
            img.metadata = image_metadata::from_dds_header(tex.metadata);
            img.metadata.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            //
            img.create_image(img.metadata, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            {
               auto  staging = this->create_buffer(tex.pixel_data_size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
               void* data    = staging.map_memory();
               memcpy(data, tex.pixel_data(), tex.pixel_data_size());
               staging.unmap_memory(data);
               //
               img.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
               img.copy_content_from_buffer(staging.handle);
               img.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
            img.create_basic_view(img.metadata.format, VK_IMAGE_ASPECT_COLOR_BIT);
         } catch (std::runtime_error& e) {
            qDebug("[vulkanDK::scene_renderer::add_texture] Exception thrown while trying to create a new texture.");
            target.content.teardown();
            target.mark_for_delete();
            return fail;
         }
         //
         // Set scene texture as out of date:
         //
         target.handled_frames.set_all_out_of_date();
         return texture_index;
      }
      //
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
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_texture] Creating new texture at index %u for texture path <%s>", texture_index, qUtf8Printable(texture_path));
      }
      auto& target = list[texture_index];
      target.life_state = scene_frame_item_state::active;
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
         {
            .extent = {
               .width  = w,
               .height = h,
            },
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         },
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      target.content.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      target.content.copy_content_from_buffer(staging.handle);
      target.content.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      target.content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      target.handled_frames.set_all_out_of_date();
      return texture_index;
   }
   size_t surface_renderer::add_dds_texture(QString texture_path) {
      constexpr size_t fail = std::string::npos;
      //
      if (!texture_path.endsWith(".dds", Qt::CaseInsensitive))
         return fail;
      //
      auto& list = this->scene.textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         if (list[i].path == texture_path) {
            if constexpr (debug_log_scene_object_lifetimes) {
               qDebug("[vulkanDK::scene_renderer::add_dds_texture] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(texture_path));
            }
            return i;
         }
      }
      //
      std::unique_ptr<dovah::bsa_archived_file> file;
      {
         texture_path = QDir::cleanPath(texture_path);
         if (!texture_path.startsWith("textures/")) {
            qDebug("[vulkanDK::surface_renderer::add_dds_texture] Invalid texture path (doesn't start with textures folder): %s", qUtf8Printable(texture_path));
            return fail;
         }
         std::filesystem::path path = texture_path.toStdWString();
         file.reset(dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path)); // TODO: switch to `get` once we're sure the asset subsystem is constructed elsewhere
         if (!file) {
            qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to open texture: %s", qUtf8Printable(texture_path));
            return fail;
         }
      }
      dds::texture tex;
      tex.data = file->data();
      tex.size = file->size();
      //
      if (!tex.read()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to read DDS header: %s", qUtf8Printable(texture_path));
         return fail;
      }
      if (!tex.pixel_data() || !tex.pixel_data_size()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] No DDS data available: %s", qUtf8Printable(texture_path));
         return fail;
      }
      //
      // Create scene texture.
      //
      auto texture_index = this->scene.insert_new_texture();
      if (texture_index == std::string::npos) {
         qDebug("[vulkanDK::scene_renderer::add_dds_texture] Cannot add new rendered textures. Maximum has been reached.");
         return fail;
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_dds_texture] Creating new texture at index %u for texture path <%s>", texture_index, qUtf8Printable(texture_path));
      }
      auto& target = list[texture_index];
      target.life_state = scene_frame_item_state::active;
      target.w    = tex.metadata.width;
      target.h    = tex.metadata.height;
      target.path = texture_path;
      //
      // Create Vulkan data:
      //
      target.content = concrete_image(*this);
      try {
         auto& img = target.content;
         img.metadata = image_metadata::from_dds_header(tex.metadata);
         img.metadata.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
         //
         img.create_image(img.metadata, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         {
            auto  staging = this->create_buffer(tex.pixel_data_size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data    = staging.map_memory();
            memcpy(data, tex.pixel_data(), tex.pixel_data_size());
            staging.unmap_memory(data);
            //
            img.transition_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            img.copy_content_from_buffer(staging.handle);
            img.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
         }
         img.create_basic_view(img.metadata.format, VK_IMAGE_ASPECT_COLOR_BIT);
      } catch (std::runtime_error& e) {
         qDebug("[vulkanDK::scene_renderer::add_dds_texture] Exception thrown while trying to create a new texture.");
         target.content.teardown();
         target.mark_for_delete();
         return fail;
      }
      //
      // Set scene texture as out of date:
      //
      target.handled_frames.set_all_out_of_date();
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
            texture_item.mark_for_delete();
         }
         return;
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_mesh] Creating new mesh at index %u with texture index %u.", object_index, texture_index);
      }
      QSize texture_size = { (int)texture_item.w, (int)texture_item.h }; // just used to size the quad so we maintain aspect ratio
      {  // Create model
         if (!texture_size.isValid())
            texture_size = { 1, 1 };
         //
         auto& ro  = this->scene.meshes[object_index];
         auto& vib = ro.vertex_and_index_buffer;
         ro.texture_indices.diffuse = texture_index;
         ++texture_item.refcount;
         texture_item.life_state = scene_frame_item_state::active;
         //
         glm::vec3 position = {};
         {
            constexpr float radius = 5.0F;
            //constexpr float radius = 0.0F;
            //
            for (size_t j = 0; j < 3; ++j)
               position[j] = ((float)rand() / RAND_MAX) * radius - (radius / 2.0F);
         }
         //
         float hfwc = ((float)texture_size.height() / texture_size.width()) / 2; // height-for-width, centered
         ro.data.vertices = {
            vertex{
               .pos    = { -0.5f, -hfwc, 0.0 },
               .color  = { 1, 1, 1 },
               .uv     = { 1, 0 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { 0.5f, -hfwc, 0.0 },
               .color  = { 1, 1, 1 },
               .uv     = { 0, 0 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { 0.5f, hfwc, 0.0 },
               .color  = { 1, 1, 1 },
               .uv     = { 0, 1 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { -0.5f, hfwc, 0.0 },
               .color  = { 1, 1, 1 },
               .uv     = { 1, 1 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
         };
         ro.data.indices = { 0, 1, 2, 2, 3, 0 };
         ro.recalc_bounding_sphere();
         ro.shader_params.transform = glm::translate(
            glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            position
         );
         //
         VkDeviceSize buffer_size_v;
         VkDeviceSize buffer_size_i;
         VkDeviceSize buffer_size;
         ro.sizes_for_setup(buffer_size_v, buffer_size_i, buffer_size);
         //
         auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         void* data    = staging.map_memory();
         ro.setup_vib_data_at(data);
         staging.unmap_memory(data);
         //
         vib.wide_indices = ro.data.indices.type() == vertex_index_list::value_type::wide;
         vib.indices_at   = buffer_size_v;
         vib.index_count  = ro.data.indices.size();
         vib.buffer       = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         vib.buffer.copy_from(staging);
         //
         if (!ro.anim_state)
            ro.anim_state = new mesh_animation_state; // for testing: spin the mesh
         ro.life_state = scene_frame_item_state::active;
         ro.handled_frames.set_all_out_of_date();
      }
      //
      for (auto& image : this->swap_chain.images)
         image.invalidate_all_command_buffers();
   }
   void surface_renderer::remove_mesh(size_t i) {
      auto& list = this->scene.meshes;
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_mesh] Marking scene mesh %u for delete.", i);
      }
      ++this->scene.pending_deletions.meshes;
      auto& item = list[i];
      item.mark_for_delete();
      {
         auto& list = this->scene.textures;
         for (auto& ti : item.texture_indices.list) {
            if (ti < 0)
               continue;
            if (ti < list.size()) {
               auto& tex = list[ti];
               if (--tex.refcount == 0) {
                  tex.mark_for_delete();
                  ++this->scene.pending_deletions.textures;
                  if constexpr (debug_log_scene_object_lifetimes) {
                     qDebug("[vulkanDK::scene_renderer::remove_mesh] Mesh %u used texture %u which is now unused; marking the texture for delete.", i, ti);
                  }
               }
            }
            ti = -1;
         }
      }
      //
      for (auto& image : this->swap_chain.images)
         image.invalidate_all_command_buffers();
   }
   void surface_renderer::remove_last_mesh() {
      auto& list = this->scene.meshes;
      auto  size = list.size();
      if (size == 0)
         return;
      for (size_t i = size - 1; i >= 0; --i) {
         auto& item = list[i];
         if (!item.active())
            continue;
         this->remove_mesh(i);
         return;
      }
   }
   void surface_renderer::set_animation_paused(size_t i, bool paused) {
      auto& list = this->scene.meshes;
      if (i >= list.size())
         return;
      auto& item = list[i];
      if (auto* as = item.anim_state) {
         as->playing = !paused;
      }
   }
   //
   size_t surface_renderer::object_index_at(int x, int y) {
      //
      // IntelliSense DOES NOT understand GLM's vector types properly and 
      // will display them as if they aren't templated on float. Examination 
      // in the run-time debugger confirms that they are indeed floats, so 
      // ignore IntelliSense's lies!
      //
      glm::vec3 eye_position;
      glm::vec3 eye_endpoint;
      glm::vec3 eye_direction;
      {
         auto viewport = glm::vec4( 0, 0, this->surface_extent.width, this->surface_extent.height );
         //
         eye_position = glm::unProject(
            glm::vec3{ x, y, 0.0 },
            this->scene.global_state.view,
            this->scene.global_state.proj,
            viewport
         );
         eye_endpoint = glm::unProject(
            glm::vec3{ x, y, 1.0 },
            this->scene.global_state.view,
            this->scene.global_state.proj,
            viewport
         );
         eye_direction = glm::normalize(eye_endpoint - eye_position);
      }
      //
      size_t nearest  = -1;
      float  distance = std::numeric_limits<float>::max();
      auto&  list     = this->scene.meshes;
      for (size_t i = 0; i < list.size(); ++i) {
         auto& mesh = list[i];
         //
         float hit_distance;
         if (!mesh.ray_intersects(eye_position, eye_direction, hit_distance))
            continue;
         if (hit_distance < distance) {
            distance = hit_distance;
            nearest = i;
         }
      }
      return nearest;
   }
   
   namespace {
      void _ni_triangles_to_mesh_triangles(const std::vector<nifDK::Triangle>& list, rendered_mesh& mesh) {
         auto  size = list.size();
         mesh.data.indices = vertex_index_list(size * 3, uint16_t(0));
         //
         auto to = mesh.data.indices.as_thin_range();
         for (size_t i = 0; i < size; ++i) {
            auto& tri = list[i];
            to[(i * 3) + 0] = tri.vertex_indices[0];
            to[(i * 3) + 1] = tri.vertex_indices[1];
            to[(i * 3) + 2] = tri.vertex_indices[2];
         }
      }
   }
   void surface_renderer::_create_mesh_vib(rendered_mesh& mesh) {
      VkDeviceSize buffer_size_v;
      VkDeviceSize buffer_size_i;
      VkDeviceSize buffer_size;
      mesh.sizes_for_setup(buffer_size_v, buffer_size_i, buffer_size);
      //
      auto  staging = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      void* data    = staging.map_memory();
      mesh.setup_vib_data_at(data);
      staging.unmap_memory(data);
      //
      auto& vib = mesh.vertex_and_index_buffer;
      vib.wide_indices = mesh.data.indices.type() == vertex_index_list::value_type::wide;
      vib.indices_at   = buffer_size_v;
      vib.index_count  = mesh.data.indices.size();
      vib.buffer       = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      vib.buffer.copy_from(staging);
      //
      mesh.handled_frames.set_all_out_of_date();
   }
   void surface_renderer::_handle_ni_textures(rendered_mesh& mesh, nifDK::block_types::BSShaderProperty* shader) {
      size_t prior_diffuse = mesh.texture_indices.diffuse;
      size_t prior_normals = mesh.texture_indices.normals;
      //
      size_t texture_index = std::string::npos;
      size_t normals_index = std::string::npos;
      if (auto* lighting = dynamic_cast<nifDK::block_types::BSLightingShaderProperty*>(shader)) {
         if (auto* textures = lighting->texture.paths) {
            const auto& diffuse = textures->textures.diffuse;
            const auto& normals = textures->textures.normal;
            if (!diffuse.empty()) {
               texture_index = this->add_dds_texture(diffuse.c_str());
            }
            if (!normals.empty()) {
               normals_index = this->add_dds_texture(normals.c_str());
            }
         }
         //
         mesh.shader_params.specular_strength = lighting->specular.strength / 1000.0F; // NIF uses 999 for max brightness?
         mesh.shader_params.specular_color    = { lighting->specular.color.r, lighting->specular.color.g, lighting->specular.color.b };
         mesh.shader_params.specular_exponent = lighting->material.glossiness;
      } else if (auto* effect = dynamic_cast<nifDK::block_types::BSEffectShaderProperty*>(shader)) {
         const auto& texture = effect->texture.path;
         if (!texture.empty()) {
            texture_index = this->add_dds_texture(texture.c_str());
         }
      }
      mesh.texture_indices.diffuse = texture_index;
      mesh.texture_indices.normals = normals_index;
      if (texture_index != std::string::npos) {
         ++this->scene.textures[texture_index].refcount;
         if (prior_diffuse != std::string::npos)
            --this->scene.textures[prior_diffuse].refcount;
      }
      if (normals_index != std::string::npos) {
         ++this->scene.textures[normals_index].refcount;
         if (prior_normals != std::string::npos)
            --this->scene.textures[prior_normals].refcount;
      }
   }
   void surface_renderer::add_BSTriShape_mesh(nifDK::block_types::BSTriShape* data, glm::mat4 transform, size_t fallback_texture_index) {
      auto size = data->vertices.size();
      if (!size || !data->triangles.size())
         return;
      qDebug("[surface_renderer::add_BSTriShape_mesh] Handling geometry: %s...", data->name.data());
      transform = transform * data->transform.to_matrix();
      //
      auto  mesh_index = this->scene.insert_new_mesh();
      assert(mesh_index != std::string::npos);
      auto& mesh       = this->scene.meshes[mesh_index];
      //
      mesh.life_state = scene_frame_item_state::active;
      mesh.shader_params.transform = transform;
      mesh.texture_indices.diffuse = fallback_texture_index;
      ++this->scene.textures[fallback_texture_index].refcount;
      {  // Vertices
         mesh.data.vertices.resize(size);
         auto&       list = data->vertices;
         const auto& desc = data->vertex_desc;
         for (size_t i = 0; i < size; ++i) {
            auto& src = list[i];
            auto& dst = mesh.data.vertices[i];
            //
            dst.pos       = src.vertex;
            dst.color     = { src.color.r, src.color.g, src.color.b }; // TODO: support RGBA vertex colors
            dst.normal    = src.normal;
            dst.tangent   = src.tangent;
            dst.bitangent = src.bitangent;
            dst.uv        = src.uv;
         }
      }
      qDebug("[surface_renderer::add_BSTriShape_mesh] Loaded %u vertices...", size);
      {  // Triangles
         _ni_triangles_to_mesh_triangles(data->triangles, mesh);
         qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded %u triangles...", data->triangles.size());
      }
      {  // Bounding sphere
         auto& dst = mesh.data.bounding_sphere;
         auto& src = data->bounds;
         dst.center    = src.center;
         dst.radius_sq = src.radius * src.radius;
         qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded NiBound...");
      }
      //
      // Vulkan:
      //
      this->_create_mesh_vib(mesh);
      qDebug("[surface_renderer::add_BSTriShape_mesh] Vulkan setup complete for geometry: %s.", data->name.c_str());
      //
      // Texture:
      //
      if (auto* shader = data->properties.shader) {
         _handle_ni_textures(mesh, shader);
      }
   }
   void surface_renderer::add_NiGeometry_mesh(nifDK::block_types::NiGeometry* object, glm::mat4 transform, size_t fallback_texture_index) {
      auto* geom = dynamic_cast<nifDK::block_types::NiTriShape*>(object); // the NiGeometry superclass isn't enough for triangle-based rendering
      if (!geom)
         return;
      auto* data = dynamic_cast<nifDK::block_types::NiTriShapeData*>(geom->data);
      if (!data)
         return;
      auto size = data->vertices.size();
      if (!size || !data->triangles.size())
         return;
      qDebug("[surface_renderer::add_NiGeometry_mesh] Handling geometry: %s...", object->name.data());
      transform = transform * geom->transform.to_matrix();
      //
      auto  mesh_index = this->scene.insert_new_mesh();
      assert(mesh_index != std::string::npos);
      auto& mesh       = this->scene.meshes[mesh_index];
      //
      mesh.life_state = scene_frame_item_state::active;
      mesh.shader_params.transform = transform;
      mesh.texture_indices.diffuse = fallback_texture_index;
      ++this->scene.textures[fallback_texture_index].refcount;
      {  // Vertices
         mesh.data.vertices.resize(size);
         auto& vl = data->vertices;
         auto& nl = data->normals;
         auto& tl = data->tangents;
         auto& bl = data->bitangents;
         auto& cl = data->vertex_colors;
         auto& ul = data->uv_sets;
         for (size_t i = 0; i < size; ++i) {
            auto& vert = mesh.data.vertices[i];
            vert.pos = data->vertices[i];
            if (cl.size()) {
               vert.color = { cl[i].r, cl[i].g, cl[i].b };
            } else {
               vert.color = { 1.0, 1.0, 1.0 };
            }
            if (ul.size()) {
               auto& uv = ul[0];
               vert.uv = uv[i];
            } else {
               vert.uv = { 0, 0 };
            }
            if (nl.size()) {
               vert.normal = nl[i];
               if (tl.size()) {
                  assert(bl.size());
                  vert.tangent   = tl[i];
                  vert.bitangent = bl[i];
               } else {
                  vert.tangent   = { 1, 0, 0 };
                  vert.bitangent = { 0, 1, 0 };
               }
            } else {
               vert.normal = { 0, 0, 1 }; // this won't be a good default...
            }
         }
      }
      qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded %u vertices...", size);
      {  // Triangles
         _ni_triangles_to_mesh_triangles(data->triangles, mesh);
         qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded %u triangles...", data->triangles.size());
      }
      {  // Bounding sphere
         auto& dst = mesh.data.bounding_sphere;
         auto& src = data->bounds;
         dst.center    = src.center;
         dst.radius_sq = src.radius * src.radius;
         qDebug("[surface_renderer::add_NiGeometry_mesh] Loaded NiBound...");
      }
      //
      // Vulkan:
      //
      this->_create_mesh_vib(mesh);
      qDebug("[surface_renderer::add_NiGeometry_mesh] Vulkan setup complete for geometry: %s.", object->name.c_str());
      //
      // Texture:
      //
      if (auto* shader = geom->properties.shader) {
         _handle_ni_textures(mesh, shader);
      }
   }
   bool surface_renderer::add_nif(nifDK::file& model) {
      if (!model.root_node) {
         qDebug("[surface_renderer::add_nif] Model has no root node.");
         return false;
      }
      size_t mesh_count = model.root_node->count_descendants_of_type<nifDK::block_types::NiGeometry>(); // TODO: there are other mesh types e.g. NiLines
      {
         auto available = this->scene.available_mesh_count();
         if (available < mesh_count) {
            qDebug("[surface_renderer::add_nif] Not enough mesh slots available for this NIF (%u needed; %u available).", mesh_count, available);
            return false;
         }
      }
      //
      size_t texture_index;
      {
         texture_index = this->add_texture(QLatin1Literal(":/shaders/white.png"));
         if (texture_index == std::string::npos) {
            qDebug("Cannot add new rendered object: failed to add its texture.");
            return false;
         }
         auto& texture_item = this->scene.textures[texture_index];
         texture_item.life_state = scene_frame_item_state::active;
      }
      //
      auto functor = [this, texture_index](nifDK::block_types::NiAVObject* object, glm::mat4 transform) {
         //
         // Lambdas can't recursively call themselves, in part because they'd have to reference their own 
         // identifiers (not possible: the auto expression isn't "complete" at parse time, so the type 
         // is unknown) and in part because those identifiers are in a different scope (lambdas can't 
         // capture themselves).
         //
         auto impl = [this, texture_index](nifDK::block_types::NiAVObject* object, glm::mat4 transform, auto& self) -> void {
            auto* node = dynamic_cast<nifDK::block_types::NiNode*>(object);
            if (node) {
               qDebug("[surface_renderer::add_nif] Handling node: %s...", node->name.data());
               transform = transform * node->transform.to_matrix();
               for (auto* child : node->children)
                  (self)(child, transform, self);
               qDebug("[surface_renderer::add_nif] Handled node: %s.", node->name.data());
               return;
            }
            if (auto* geom = dynamic_cast<nifDK::block_types::NiGeometry*>(object)) {
               this->add_NiGeometry_mesh(geom, transform, texture_index);
            }
            if (auto* geom = dynamic_cast<nifDK::block_types::BSTriShape*>(object)) {
               this->add_BSTriShape_mesh(geom, transform, texture_index);
            }
         };
         impl(object, transform, impl);
      };
      (functor)(model.root_node, glm::mat4(1));
      qDebug("[surface_renderer::add_nif] Done processing the NIF.");
      {
         auto& tex = this->scene.textures[texture_index];
         if (tex.refcount == 0) {
            //
            // Means this model failed to produce any meshes (AND no other meshes loaded via 
            // this function did either), so the white.png texture is unused.
            //
            qDebug("[surface_renderer::add_nif] Texture is unreferenced; deleting it.");
            tex.mark_for_delete();
         }
      }
      //
      for (auto& image : this->swap_chain.images)
         image.invalidate_all_command_buffers();
      return true;
   }

   void surface_renderer::move_camera(const glm::vec3& move, const glm::vec3& turn) {
      auto& gs     = this->scene.global_state;
      auto& camera = gs.view;
      //
      auto rot = glm::eulerAngleZY(turn.z, turn.y);
      rot *= glm::eulerAngleX(turn.x);
      //
      camera *= rot;
      auto position = glm::inverse(glm::mat3x3(camera)) * move;
      camera = glm::translate(camera, position);
      /*//
      glm::mat3 rotation = camera;
      glm::vec3 position = camera[3];
      rotation *= glm::mat3(rot);
      position += glm::inverse(rotation) * move;
      camera = glm::translate(glm::mat4(rotation), position);
      //*/
   }
   void surface_renderer::set_camera_position(const glm::vec3& position) {
      this->scene.camera.position = position;
      this->scene.update_camera();
   }

   void surface_renderer::_execute_pending_scene_deletions() {
      auto  ic = this->swap_chain.images.size();
      auto& pd = this->scene.pending_deletions;
      if (pd.meshes) {
         size_t deleted    =  0;
         size_t last_alive = -1;
         auto&  list       = this->scene.meshes;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date(ic)) {
               item.reset();
               ++deleted;
            } else {
               last_alive = i;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            if (deleted) {
               qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_deletions] Deleted %u scene meshes.", deleted);
            }
         }
         pd.meshes -= deleted;
         list.resize(last_alive + 1);
      }
      if (pd.textures) {
         size_t deleted    =  0;
         size_t last_alive = -1;
         auto&  list       = this->scene.textures;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date(ic)) {
               item.reset();
               ++deleted;
            } else {
               last_alive = i;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            if (deleted) {
               qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_deletions] Deleted %u scene textures.", deleted);
            }
         }
         pd.textures -= deleted;
         list.resize(last_alive + 1);
      }
   }
   #pragma endregion
}