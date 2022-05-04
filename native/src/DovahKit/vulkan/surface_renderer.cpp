#include "surface_renderer.h"
#include <chrono>
#include <typeinfo>
#include <QResource> // for loading shaders
#include "helpers/array_concat.h"
//
#include "DKVulkanInstance.h"
#include "compute_shader.h"
#include "exceptions.h"
#include "frame_in_flight.h"
#include "physical_device.h"
#include "queue_family_info.h"
#include "render_pass.h"
#include "shader_module.h"
#include "vertex.h"
#include "config/frames_in_flight.h"
#include "config/scene_limits.h"
#include "config/shadow_maps.h"
#include "config/use_inverted_depth.h"
#include "config/validation_layers.h"
#include "helpers/convert_access_flags_and_pipeline_stages.h"
#include "helpers/cubemap_helpers.h"
#include "helpers/glm_transform_from_beth.h"
#include "helpers/specialization_map_entry_for_member.h"

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
#include "nif/blocks/NiAlphaProperty.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiSwitchNode.h"
#include "nif/blocks/NiTriShape.h"
#include "nif/blocks/NiTriShapeData.h"

// loading refs and landscapes
#include "dovah/form_stub.h"
#include "dovah/form_stub_helpers.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/LandTexture.h"
#include "dovah/forms/Light.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/forms/TextureSet.h"
#include "dovah/forms/components/extra_data/light.h"
#include "dovah/forms/components/extra_data/radius.h"

namespace {
   static constexpr bool debug_log_scene_object_lifetimes   = false;
   static constexpr bool debug_object_names_fallback_to_log = false; // logs objects' debug names when the relevant extension isn't supported; log spam on window resize; use only when needed
}

namespace {
   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
      VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
   };

   static constexpr auto desired_swap_chain_presentation_mode  = VK_PRESENT_MODE_MAILBOX_KHR;
   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;
}

#include "overlays/fps.h"
namespace {
   static constexpr bool setup_fps_counter = true; // mainly just used for grouping code, tbh
}

namespace {
   static constexpr VkFormat format_for_oit_accumulator = VK_FORMAT_R16G16B16A16_SFLOAT;
   static constexpr VkFormat format_for_oit_reveal      = VK_FORMAT_R16_SFLOAT;
}

namespace {
   constexpr std::array<vulkanDK::vertex, 4> _make_quad(float hfwc, bool test_colors) { // height-for-width, centered
      using namespace vulkanDK;
      //
      std::array<vulkanDK::vertex, 4> out = {};
      for (size_t i = 0; i < out.size(); ++i) {
         auto& v = out[i];
         v.color = { 1, 1, 1, 1 };
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
   /*static*/ bool surface_renderer::device_is_supported(const physical_device& pd) {
      if (pd.support.vulkan_api_version < VK_API_VERSION_1_1)
         return false;
      if (!pd.support.descriptor_bindings.runtime_array)
         return false;
      if (!pd.support.descriptor_bindings.variable_count)
         return false;
      if (!pd.support.multiview.available)
         return false;
      if (!pd.support.uniform_buffer_std430_layout)
         return false;
      if (!pd.has_extensions(device_extensions))
         return false;
      return true;
   }
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
      this->descriptor_set_layouts.scene_state.bindings = {
         vulkanDK::descriptor_binding{ // uniform buffer object: vulkanDK::scene_global_state
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.shadow_caster_map_render.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: mat4[shadow_caster_count][6]
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.shadow_maps.bindings = {
         vulkanDK::descriptor_binding{ // sun shadow map
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // light shadow maps
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count              = shadow_caster_count,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.all_textures.bindings = {
         vulkanDK::descriptor_binding{ // texture sampler
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_SAMPLER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // texture array
            .index              = 1,
            .flags              = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            .type               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .count              = config::max_loaded_textures,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.all_bounds.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: glm::mat4[] (bound matrices)
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.all_landscapes.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_landscape::shader_parameters[]
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.all_lights.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_light::shader_parameters[]
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.all_meshes.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_mesh::shader_parameters[]
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.overlay_fps.bindings = {
         vulkanDK::descriptor_binding{ // screen size
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // texture atlas
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.overlay_world_axes.bindings = {
         vulkanDK::descriptor_binding{ // world camera info
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.shared_layouts.compute_cull_caster.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_mesh::cull_data[]
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: vec4[] (XYZ position and radius of all active casters)
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: uint32_t[] (mesh indices)
            .index              = 2,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: VkDrawIndexedIndirectCommand[]
            .index              = 3,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.shared_layouts.compute_cull_frustum.bindings = {
         vulkanDK::descriptor_binding{ // storage buffer object: glm::vec4[4][] (frustum normal vectors)
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: rendered_mesh::cull_data[]
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: uint32_t[] (mesh indices)
            .index              = 2,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{ // storage buffer object: VkDrawIndexedIndirectCommand[]
            .index              = 3,
            .type               = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_COMPUTE_BIT,
            .immutable_samplers = nullptr,
         },
      };
      this->descriptor_set_layouts.oit_compositing.bindings = {
         vulkanDK::descriptor_binding{
            .index              = 0,
            .type               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
            .immutable_samplers = nullptr,
         },
         vulkanDK::descriptor_binding{
            .index              = 1,
            .type               = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            .count              = 1,
            .shader_stages      = VK_SHADER_STAGE_FRAGMENT_BIT,
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
      {
         this->api_functions.vkSetDebugUtilsObjectNameEXT = nullptr;
      }
      //
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
      auto uniform_std430 = VkPhysicalDeviceUniformBufferStandardLayoutFeatures{
         .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES,
         .pNext = nullptr,
         .uniformBufferStandardLayout = VK_TRUE,
      };
      auto multiview = VkPhysicalDeviceMultiviewFeatures{
         .sType     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
         .pNext     = &uniform_std430,
         .multiview = VK_TRUE, // NOTE: failing to enable this here causes validation layers to emit misleading VUID 01091
      };
      auto device_features = VkPhysicalDeviceFeatures2{
         .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
         .pNext    = &multiview,
         .features = {
            .independentBlend  = pd_support.independent_blending ? VK_TRUE : VK_FALSE,
            .fillModeNonSolid  = pd_support.non_solid_polygon_fill_modes ? VK_TRUE : VK_FALSE,
            .wideLines         = pd_support.wide_lines.available ? VK_TRUE : VK_FALSE,
            .largePoints       = pd_support.large_points ? VK_TRUE : VK_FALSE,
            .samplerAnisotropy = pd_support.max_anisotropic_filtering > 0 ? VK_TRUE : VK_FALSE,
         },
      };
      auto robustness_extensions = VkPhysicalDeviceRobustness2FeaturesEXT{
         .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
         .pNext               = &device_features,
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
      auto create_ext  = device_extensions;
      auto create_info = VkDeviceCreateInfo{
         .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
         .pNext                   = &indexing_extensions,
         .queueCreateInfoCount    = (uint32_t)queue_infos.size(),
         .pQueueCreateInfos       = queue_infos.data(),
         .enabledExtensionCount   = (uint32_t)create_ext.size(),
         .ppEnabledExtensionNames = create_ext.data(),
         .pEnabledFeatures        = nullptr, // specified via device_features, in the pNext chain
      };
      if (config::enable_validation_layers) {
         create_info.enabledLayerCount   = static_cast<uint32_t>(config::desired_validation_layers.size());
         create_info.ppEnabledLayerNames = config::desired_validation_layers.data();
         //
         if (this->device_info->has_extension("VK_EXT_debug_marker")) {
            create_ext.push_back("VK_EXT_debug_marker");
            create_info.enabledExtensionCount   = (uint32_t)create_ext.size();
            create_info.ppEnabledExtensionNames = create_ext.data();
         }
      } else {
         create_info.enabledLayerCount = 0;
      }
      //
      if (auto result = vkCreateDevice(pd_handle, &create_info, nullptr, &this->logical_device); result != VK_SUCCESS) {
         // report VK_ERROR_DEVICE_LOST
         throw result_exception(result, "[vulkanDK::surface_renderer] Failed to create logical device.");
      }
      //
      // And lastly, let's get our queues:
      //
      this->queues.compute.setup_handle(this->logical_device, indices.families.compute);
      this->queues.graphics.setup_handle(this->logical_device, indices.families.graphics);
      this->queues.presentation.setup_handle(this->logical_device, indices.families.presentation);
      this->queues.transfer.setup_handle(this->logical_device, indices.families.transfer);
      {
         auto& list = this->queues.list;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& a = list[i];
            for (size_t j = i + 1; j < list.size(); ++j) {
               auto& b = list[j];
               if (a.index == b.index) {
                  //
                  // These two queue entries actually refer to the same underlying queue. We'll set one as 
                  // an "alias" of the other, so that we don't redundantly create multiple command pools 
                  // for the same queue.
                  //
                  if (&b == &this->queues.graphics)
                     a.set_alias_of(b);
                  else
                     b.set_alias_of(a);
               }
            }
         }
      }
      //
      // Oh, and some niche API functions:
      //
      if (this->device_info->has_extension("VK_EXT_debug_marker")) {
         this->api_functions.vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(this->logical_device, "vkSetDebugUtilsObjectNameEXT");
      }
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
      if (auto result = vkCreateSampler(this->logical_device, &sampler_info, nullptr, &this->raw_pixel_texture_sampler); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::surface_renderer::_setup_raw_pixel_texture_sampler] Failed to create the raw pixel texture sampler.");
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
      {
         auto fence_info = VkFenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
         };
         vkCreateFence(this->logical_device, &fence_info, nullptr, &this->one_time_commands_fence);
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
      //
      this->descriptor_set_layouts.setup_all(*this);
      this->_define_render_passes();
      this->_setup_shaders();
      this->setup_texture_sampler(); // descriptor set layout must be able to refer to our immutable sampler
      this->_setup_raw_pixel_texture_sampler();
      //
      for (auto& q : this->queues.list)
         q.setup_command_pools(this->logical_device);
      //
      this->scene.setup_landscape_buffer(*this, config::max_landscapes);
      {  // swap chain
         this->_setup_sun_shadow_buffer();
         this->_setup_swap_chain_instance();         // sets up format, extent size, and handle
         this->_setup_render_passes();               // requires swap chain format
         for (auto* s : this->graphics_shaders)
            s->setup_pipeline(this->surface_extent);
         this->_setup_oit_images();                  // requires extent size
         this->_setup_depth_buffer();                // requires extent size
         this->_setup_color_buffer();                // requires extent size
         this->_setup_frames_in_flight();
         this->_setup_swap_chain_images();
         this->_setup_framebuffers();                // requires extent size
         this->_setup_descriptor_pool();             // requires swap chain image count
         for (auto& fif : this->swap_chain.frames_in_flight)
            fif.setup_descriptor_sets();
      }
      this->_setup_light_shadow_resources();
      this->_create_null_texture();
      this->scene.update_projection(this->surface_extent); // requires extent size
      this->_setup_initial_scene();
      this->_initialize_descriptor_sets();
      //
      this->_on_renderer_ready();
   }
   void surface_renderer::_define_render_passes() {
      {
         auto* rp = this->render_passes_by_name.main_shadow = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            },
         };
         rp->subpasses = {
            {  // subpass
               .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 0,
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // wait until the fragment shader
               .srcAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      {
         auto* rp = this->render_passes_by_name.main_shadow_placed = new render_pass(*this);
         {
            auto desc = VkAttachmentDescription{
               .format         = VK_FORMAT_R32_SFLOAT,
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE, // we won't use this data after subpass 0, where it's generated, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            //
            auto& list = rp->attachments;
            list.resize(shadow_caster_count);
            for(auto& item : list)
               item = desc;
         }
         {
            auto& list = rp->subpasses;
            list.resize(shadow_caster_count);
            for (size_t i = 0; i < shadow_caster_count; ++i) {
               list[i] = {  // subpass
                  .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                  .attachments = {
                     .color = {
                        VkAttachmentReference{
                           .attachment = (uint32_t)i,
                           .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        },
                     },
                  },
                  .view_mask = 0b00111111,
               };
            }
         }
         rp->subpass_dependencies = {
            VkSubpassDependency{
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = 0,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            },
         };
      }
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
               //.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // for OIT
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses = {
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
         rp->subpass_dependencies = {
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
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, // should be the destination of the last dependency?
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }
         };
      }
      if (this->can_do_alpha()) {
         auto* rp = this->render_passes_by_name.main_oit = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // OIT accumulator
               .format         = format_for_oit_accumulator,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // OIT reveal
               .format         = format_for_oit_reveal,
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // when we finish, don't bother changing the image layout (i.e. "set" it to the layout of the depth-stencil image, which it is)
            },
         };
         rp->subpasses = {
            {  // subpass: color pass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = {
                     VkAttachmentReference{ // OIT accumulator
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     },
                     VkAttachmentReference{ // OIT reveal
                        .attachment = 1,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     }
                  },
                  .depth_stencil = VkAttachmentReference{ // there can only be one depth/stencil attachment
                     .attachment = 3,
                     .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                  },
               },
            },
            {  // subpass: composite pass
               .bind_point  = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .attachments = {
                  .color = {
                     VkAttachmentReference{ // color
                        .attachment = 2,
                        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     },
                  },
                  .input = {
                     VkAttachmentReference{ // OIT accumulator
                        .attachment = 0,
                        .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     },
                     VkAttachmentReference{ // OIT reveal
                        .attachment = 1,
                        .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     }
                  },
               },
            },
         };
         rp->subpass_dependencies = {
            VkSubpassDependency{
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = 0, // 0 == all operations? documentation/spec are unclear
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0,
               .dstSubpass      = 1,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{ // dependency to transition the images back to optimal
               .srcSubpass      = 1,
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
         };
      }
      {
         auto* rp = this->render_passes_by_name.bounds = new render_pass(*this);
         rp->attachments = { // ordered list; indices are referred to in the "attachment references" within subpass descriptions
            VkAttachmentDescription{ // color
               .format         = VK_FORMAT_UNDEFINED,   // This needs to be set to the swap chain image format; see _setup_render_passes.
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
            VkAttachmentDescription{ // depth
               .format         = this->find_depth_format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT, // related to multisampling
               .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE, // we won't use this data after this render pass, so let the driver decide how best to discard it
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            },
         };
         rp->subpasses = {
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
         rp->subpass_dependencies = {
            VkSubpassDependency{
               //
               // Writing to the color attachment image  should be delayed until all operations 
               // in the  "external" subpass  (e.g. transitioning to the desired initial  image 
               // layout) are complete.
               //
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = 0,
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
               .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
            },
            VkSubpassDependency{
               .srcSubpass      = 0, // should be the last subpass in the list
               .dstSubpass      = VK_SUBPASS_EXTERNAL,
               .srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // wait until full command buffer is done
               .srcAccessMask   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = 0,
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
               .initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
         rp->subpasses = {
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
         rp->subpass_dependencies = {
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
      this->render_passes = {
         this->render_passes_by_name.main_shadow,
         this->render_passes_by_name.main_shadow_placed,
         this->render_passes_by_name.main,
         this->render_passes_by_name.bounds,
         this->render_passes_by_name.ui,
         this->render_passes_by_name.main_oit,
      };
   }

   void surface_renderer::_setup_oit_composite_shader() {
      if (!this->can_do_alpha()) {
         //
         // If the device doesn't support the features we need for OIT, then we don't even 
         // define the render pass, so we also shouldn't load any shaders that rely on it.
         //
         return;
      }
      assert(this->render_passes_by_name.main_oit != nullptr);
      //
      auto* s = this->create_graphics_shader(oit_composite_shader_id);
      s->set_render_pass(this->render_passes_by_name.main_oit, 1);
      s->set_layout_info({ this->descriptor_set_layouts.oit_compositing.handle });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/util/full-screen-triangle.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/util/oit-composite.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (OIT Composite: util/full-screen-triangle.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (OIT Composite: util/oit-composite.frag.spv)");
      }
      //
      options.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE; // the vertex shader produces a clockwise triangle
      options.stages = {
         {
            .module           = frag,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .module           = vert,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_VERTEX_BIT,
         },
      };
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{
         .source = {
            .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
         },
         .destination = {
            .color = VK_BLEND_FACTOR_SRC_ALPHA,
            .alpha = VK_BLEND_FACTOR_SRC_ALPHA,
         },
      });
      options.depth.testing    = true;
      options.depth.writing    = false;
      options.depth.comparison = VK_COMPARE_OP_ALWAYS;
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_rendered_mesh_shaders() {
      this->_setup_rendered_mesh_color_shader();
      this->_setup_rendered_mesh_wboit_shader();
      this->_setup_rendered_mesh_shadows_caster_shaders();
      this->_setup_rendered_mesh_shadows_sun_shader();
   }
   void surface_renderer::_setup_rendered_mesh_color_shader() {
      auto* s = this->create_graphics_shader(main_shader_id);
      s->set_render_pass(this->render_passes_by_name.main);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_textures.handle,
            this->descriptor_set_layouts.all_meshes.handle,
            this->descriptor_set_layouts.all_lights.handle,
            this->descriptor_set_layouts.shadow_maps.handle,
         },
         {  // Push constants
            VkPushConstantRange{
               .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
               .offset     = 0,
               .size       = sizeof(rendered_mesh::push_constant),
            }
         }
      );
      s->add_variant({ // double-sided shader variant
         .face_cull_mode = VK_CULL_MODE_NONE,
      });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/color.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/color-main.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Color Main: rendered_mesh/bslp/color.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Color Main: rendered_mesh/bslp/color-main.frag.spv)");
      }
      //
      struct _specializations {
         int32_t max_lights = config::max_rendered_lights;
      };
      _specializations spec;
      //
      options.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
            .specialization_info = pipeline_stage_specialization_info((int32_t)config::max_rendered_lights),
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
            .specialization_info = pipeline_stage_specialization_info((int32_t)config::max_rendered_lights),
         },
      };
      //dfn.color_blending.blends.emplace_back(graphics_shader::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
      options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
      if constexpr (config::use_inverted_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER;
      }
      options.rasterization.depthBiasEnable = VK_TRUE; // needed so we can selectively use depth bias during rendering; we'll leave the actual settings at 0, which is functionally off
      options.dynamic_states = {
         VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS, // for decals
      };
      {
         auto& vertex     = options.inputs.vertex;
         auto  attributes = vertex::getAttributeDescriptions();
         vertex.bindings.push_back(vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_rendered_mesh_wboit_shader() {
      if (!this->can_do_alpha()) {
         //
         // If the device doesn't support the features we need for OIT, then we don't even 
         // define the render pass, so we also shouldn't load any shaders that rely on it.
         //
         return;
      }
      assert(this->render_passes_by_name.main_oit != nullptr);
      //
      auto* s = this->create_graphics_shader(main_shader_oit_color_id);
      s->set_render_pass(this->render_passes_by_name.main_oit, 0);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_textures.handle,
            this->descriptor_set_layouts.all_meshes.handle,
            this->descriptor_set_layouts.all_lights.handle,
            this->descriptor_set_layouts.shadow_maps.handle,
         },
         {  // Push constants
            VkPushConstantRange{
               .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
               .offset     = 0,
               .size       = sizeof(rendered_mesh::push_constant),
            }
         }
      );
      s->add_variant({ // double-sided shader variant
         .face_cull_mode = VK_CULL_MODE_NONE,
      });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/color.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/color-oit.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Color WBOIT: rendered_mesh/bslp/color.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Color WBOIT: rendered_mesh/bslp/color-oit.frag.spv)");
      }
      //
      options.stages = {
         {
            .module              = frag,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
            .specialization_info = pipeline_stage_specialization_info(
               (int32_t)config::max_rendered_lights
            ),
         },
         {
            .module              = vert,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_VERTEX_BIT,
            .specialization_info = pipeline_stage_specialization_info(
               (int32_t)config::max_rendered_lights
            ),
         },
      };
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // accumulator
         .source = {
            .color = VK_BLEND_FACTOR_ONE,
            .alpha = VK_BLEND_FACTOR_ONE,
         },
         .destination = {
            .color = VK_BLEND_FACTOR_ONE,
            .alpha = VK_BLEND_FACTOR_ONE,
         },
         .operations = {
            .color = VK_BLEND_OP_ADD,
            .alpha = VK_BLEND_OP_ADD,
         },
      });
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{ // reveal
         .source = {
            .color = VK_BLEND_FACTOR_ZERO,
            .alpha = VK_BLEND_FACTOR_ZERO,
         },
         .destination = {
            .color = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
            .alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
         },
         .operations = {
            .color = VK_BLEND_OP_ADD,
            .alpha = VK_BLEND_OP_ADD,
         },
      });
      options.depth.testing = true;
      options.depth.writing = false;
      if constexpr (config::use_inverted_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER;
      }
      options.rasterization.depthBiasEnable = VK_TRUE; // needed so we can selectively use depth bias during rendering; we'll leave the actual settings at 0, which is functionally off
      options.dynamic_states = {
         VkDynamicState::VK_DYNAMIC_STATE_DEPTH_BIAS, // for decals
      };
      {
         auto& vertex     = options.inputs.vertex;
         auto  attributes = vertex::getAttributeDescriptions();
         vertex.bindings.push_back(vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      //
      // And be sure to set up the pipeline layout when you're done!
      //
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_rendered_mesh_shadows_sun_shader() {
      auto* s = this->create_graphics_shader(sun_shadow_shader_id);
      s->set_render_pass(this->render_passes_by_name.main_shadow);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_meshes.handle,
            this->descriptor_set_layouts.all_textures.handle,
         },
         {  // Push constants
            VkPushConstantRange{
               .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
               .offset     = 0,
               .size       = sizeof(rendered_mesh::push_constant),
            }
         }
      );
      s->add_variant({ // double-sided shader variant
         .face_cull_mode = VK_CULL_MODE_NONE,
      });
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-sun.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-sun.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Sun Shadow: rendered_mesh/bslp/shadows-sun.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (Sun Shadow: rendered_mesh/bslp/shadows-sun.frag.spv)");
      }
      //
      options.stages = {
         {
            .module           = vert,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_VERTEX_BIT,
         },
         {
            .module           = frag,
            .entry_point_name = "main",
            .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
      };
      if constexpr (config::sun_shadow_invert_culling) {
         options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
      }
      options.rasterization.depthBiasEnable         = VK_TRUE;
      options.rasterization.depthBiasConstantFactor = 1.25F;
      options.rasterization.depthBiasSlopeFactor    = 1.75F;
      options.rasterization.depthBiasClamp          = 0.00F;
      options.color_blending.blends.emplace_back(graphics_shader::color_blend{}); // add a default blend: a disabled, "draw the source directly onto the destination" RGBA blend.
      if constexpr (config::sun_shadow_invert_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER_OR_EQUAL;
      } else {
         options.depth.comparison = VK_COMPARE_OP_LESS_OR_EQUAL;
      }
      {
         auto& vertex     = options.inputs.vertex;
         auto  attributes = vertex::getAttributeDescriptions();
         vertex.bindings.push_back(vertex::getBindingDescription());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      options.area = {
         .mode = graphics_shader::area_mode::custom,
         .scissor = {
            .offset = { .x = 0, .y = 0 },
            .extent = {
               .width  = config::sun_shadow_map_resolution_x,
               .height = config::sun_shadow_map_resolution_y,
            },
         },
         .viewport = {
            .x        = 0,
            .y        = 0,
            .width    = config::sun_shadow_map_resolution_x,
            .height   = config::sun_shadow_map_resolution_y,
            .minDepth = 0.0,
            .maxDepth = 1.0,
         },
      };
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_rendered_mesh_shadows_caster_shaders() {
      shader_module* vert = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-caster.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/rendered_mesh/bslp/shadows-caster.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_mesh/bslp/shadows-caster.vert.spv)");
         this->set_debug_object_name(frag->handle, "Shader Module (Rendered Mesh Shadows/Caster: rendered_mesh/bslp/shadows-caster.frag.spv)");
      }
      //
      static_assert(shadow_caster_count < 10, "The way we generate shader IDs here won't work for 10 or more shadow casters.");
      for (size_t i = 0; i < shadow_caster_count; ++i) {
         auto id = light_shadow_map_shader_base_id;
         id.bytes[7] += i;
         //
         auto* s = this->create_graphics_shader(id);
         s->set_render_pass(this->render_passes_by_name.main_shadow_placed, i);
         s->set_layout_info(
            {  // Descriptor set layouts
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_textures.handle,
            this->descriptor_set_layouts.all_meshes.handle,
            this->descriptor_set_layouts.all_lights.handle,
            this->descriptor_set_layouts.shadow_caster_map_render.handle,
            },
            {  // Push constants
               VkPushConstantRange{
                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                  .offset     = 0,
                  .size       = sizeof(rendered_mesh::push_constant),
               }
            }
         );
         s->add_variant({ // double-sided shader variant
            .face_cull_mode = VK_CULL_MODE_NONE,
         });
         //
         auto& options = s->options;
         options.stages = {
            {
               .module              = vert,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_VERTEX_BIT,
               .specialization_info = pipeline_stage_specialization_info((int32_t)i, (int32_t)config::max_rendered_lights),
            },
            {  // Fragment shader needed to discard alpha-tested pixels
               .module              = frag,
               .entry_point_name    = "main",
               .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
         };
         if constexpr (config::light_shadow_invert_culling) {
            options.rasterization.cullMode = VK_CULL_MODE_FRONT_BIT;
         }
         options.rasterization.depthBiasEnable         = VK_TRUE;
         options.rasterization.depthBiasConstantFactor = 1.25F;
         options.rasterization.depthBiasSlopeFactor    = 1.75F;
         options.rasterization.depthBiasClamp          = 0.00F;
         options.rasterization.frontFace               = cubemaps_are_lefthanded ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE; // cubemaps are lefthanded in Vulakn, borrowing OpenGL conventions
         options.color_blending.blends.emplace_back(graphics_shader::color_blend{
            .enabled = true,
            .operations = {
               .color = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
               .alpha = config::light_shadow_invert_depth ? VK_BLEND_OP_MAX : VK_BLEND_OP_MIN,
            },
         });
         options.depth.comparison = VK_COMPARE_OP_ALWAYS;
         //
         {
            auto& vertex     = options.inputs.vertex;
            auto  attributes = vertex::getAttributeDescriptions();
            vertex.bindings.push_back(vertex::getBindingDescription());
            vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
         }
         options.area = {
            .mode = graphics_shader::area_mode::custom,
            .scissor = {
               .offset = { .x = 0, .y = 0 },
               .extent = {
                  .width  = config::light_shadow_map_resolution_x,
                  .height = config::light_shadow_map_resolution_y,
               },
            },
            .viewport = {
               .x        = 0,
               .y        = 0,
               .width    = config::light_shadow_map_resolution_x,
               .height   = config::light_shadow_map_resolution_y,
               .minDepth = 0.0,
               .maxDepth = 1.0,
            },
         };
         //
         // And be sure to set up the pipeline layout when you're done!
         //
         s->setup_pipeline_layout();
      }

   }
   void surface_renderer::_setup_frustum_cull_shader() {
      auto* s = this->create_compute_shader(frustum_cull_shader_id);
      s->set_layout_info(
         {  // Descriptor set layouts
            this->descriptor_set_layouts.shared_layouts.compute_cull_frustum.handle,
         }
      );
      shader_module* comp = this->load_shader_module("shaders/compute/culling/frustum.comp.spv");
      {
         assert(comp);
         this->set_debug_object_name(comp->handle, "Shader Module (Frustum Cull: compute/culling/frustum.comp.spv)");
      }
      s->config.stage = pipeline_stage_info{
         .module              = comp,
         .entry_point_name    = "main",
         .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
         .specialization_info = pipeline_stage_specialization_info(
            (int32_t)config::max_rendered_meshes//,
         ),
      };
      s->setup(*this);
   }
   void surface_renderer::_setup_shadow_caster_cull_shaders() {
      static_assert(config::max_active_shadow_casters < 9, "if we want more than 10 shadow casters, then we need to change how we generate these shader IDs");
      for (size_t i = 0; i < config::max_active_shadow_casters; ++i) {
         auto id = shadow_caster_cull_shader_base_id;
         id.bytes[7] += i;
         //
         auto* s = this->create_compute_shader(id);
         s->set_layout_info(
            {  // Descriptor set layouts
               this->descriptor_set_layouts.shared_layouts.compute_cull_caster.handle,
            }
         );
         shader_module* comp = this->load_shader_module("shaders/compute/culling/shadows-caster.comp.spv");
         {
            assert(comp);
            this->set_debug_object_name(comp->handle, "Shader Module (Shadow Caster Cull: compute/culling/shadows-caster.comp.spv)");
         }
         s->config.stage = pipeline_stage_info{
            .module              = comp,
            .entry_point_name    = "main",
            .stage               = VK_SHADER_STAGE_COMPUTE_BIT,
            .specialization_info = pipeline_stage_specialization_info(
               (int32_t)config::max_rendered_meshes,
               (int32_t)config::max_active_shadow_casters,
               (int32_t)i//,
            ),
         };
         s->setup(*this);
      }
   }
   void surface_renderer::_setup_scene_bounds_shaders() {
      #pragma region Bounding box
      {
         auto* s = this->create_graphics_shader(bounding_box_shader_id);
         s->set_render_pass(this->render_passes_by_name.bounds);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_bounds.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_bounds/box/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_bounds/box/color.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Bounding Box Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Bounding Box Frag)");
         }
         //
         options.stages = {
            {
               .module           = frag,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
               .module           = vert,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
         options.rasterization.cullMode    = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         }
         //
         s->setup_pipeline_layout();
      }
      #pragma endregion
      #pragma region Pivot
      {
         auto* s = this->create_graphics_shader(bounding_origin_shader_id);
         s->set_render_pass(this->render_passes_by_name.bounds);
         s->set_layout_info({
            this->descriptor_set_layouts.scene_state.handle,
            this->descriptor_set_layouts.all_bounds.handle,
         });
         //
         auto& options = s->options;
         //
         shader_module* vert = this->load_shader_module("shaders/rendered_bounds/pivot/color.vert.spv");
         shader_module* frag = this->load_shader_module("shaders/rendered_bounds/pivot/color.frag.spv");
         {
            assert(vert);
            assert(frag);
            this->set_debug_object_name(vert->handle, "Shader Module (Bounding Box Pivot Vert)");
            this->set_debug_object_name(frag->handle, "Shader Module (Bounding Box Pivot Frag)");
         }
         //
         options.stages = {
            {
               .module           = frag,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
               .module           = vert,
               .entry_point_name = "main",
               .stage            = VK_SHADER_STAGE_VERTEX_BIT,
            },
         };
         options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
         if constexpr (config::use_inverted_depth) {
            options.depth.comparison = VK_COMPARE_OP_GREATER;
         }
         options.inputs.triangles.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
         options.rasterization.cullMode    = VK_CULL_MODE_NONE;
         if (this->device_info->support.non_solid_polygon_fill_modes) {
            options.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
         }
         //
         s->setup_pipeline_layout();
      }
      #pragma endregion
   }
   void surface_renderer::_setup_landscape_shader() {
      auto* s = this->create_graphics_shader(landscape_shader_id);
      s->set_render_pass(this->render_passes_by_name.main);
      s->set_layout_info({
         this->descriptor_set_layouts.scene_state.handle,
         this->descriptor_set_layouts.all_textures.handle,
         this->descriptor_set_layouts.all_landscapes.handle,
         this->descriptor_set_layouts.all_lights.handle,
      });
      //
      auto& options = s->options;
      //
      shader_module* vert = this->load_shader_module("shaders/rendered_landscape/color.vert.spv");
      shader_module* frag = this->load_shader_module("shaders/rendered_landscape/color.frag.spv");
      {
         assert(vert);
         assert(frag);
         this->set_debug_object_name(vert->handle, "Shader Module (Landscape Vert)");
         this->set_debug_object_name(frag->handle, "Shader Module (Landscape Frag)");
      }
      //
      options.stages = {
         {
            .module = frag,
            .entry_point_name = "main",
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .module = vert,
            .entry_point_name = "main",
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
         },
      };
      options.color_blending.blends.emplace_back(graphics_shader::default_alpha_blend); // needed for alpha testing to work
      if constexpr (config::use_inverted_depth) {
         options.depth.comparison = VK_COMPARE_OP_GREATER;
      }
      {
         auto& vertex     = options.inputs.vertex;
         auto  attributes = vertex_landscape::attribute_descriptions();
         vertex.bindings.push_back(vertex_landscape::binding_description());
         vertex.attributes.insert(vertex.attributes.end(), attributes.begin(), attributes.end());
      }
      //
      s->setup_pipeline_layout();
   }
   void surface_renderer::_setup_shaders() {
      this->_setup_oit_composite_shader();
      //
      this->_setup_rendered_mesh_shaders();
      this->_setup_frustum_cull_shader();
      this->_setup_shadow_caster_cull_shaders();
      this->_setup_scene_bounds_shaders();
      this->_setup_landscape_shader();
      //
      // FPS counter:
      //
      if constexpr (setup_fps_counter) {
         vulkanDK::overlays::fps::setup_shaders(*this);
      }
      vulkanDK::overlays::world_axes::setup_shaders(*this);
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
         nt.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
      }
      //
      nt.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      this->set_debug_object_name(nt.handle, "Null Texture");
      this->set_debug_object_name(nt.view,   "Null Texture View");
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
               throw exception("[vulkanDK::surface_renderer::_setup_initial_scene] Failed to load test image.");
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
            target.content = owned_image_and_view(*this);
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
            target.content.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
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
                     .color  = { 1.0, 0.6, 0.0, 1.0 },
                     .uv     = { 1, 0 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { size, -size, 0 },
                     .color  = { 1.0, 0.6, 0.0, 1.0 },
                     .uv     = { 0, 0 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { size, size, 0 },
                     .color  = { 1.0, 0.6, 0.0, 1.0 },
                     .uv     = { 0, 1 },
                     .normal    = { 0, 0, 1 },
                     .tangent   = { 1, 0, 0 },
                     .bitangent = { 0, 1, 0 },
                  },
                  {
                     .pos    = { -size, size, 0 },
                     .color  = { 1.0, 0.6, 0.0, 1.0 },
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
               .sampler     = VK_NULL_HANDLE,
               .imageView   = list[i].content.view,
               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
         }
      }
      //
      auto image_info_sun_shadow = VkDescriptorImageInfo{
         .sampler     = this->canvas.sun_shadow.sampler,
         .imageView   = this->canvas.sun_shadow.map.view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      auto image_info_oit_accumulator = VkDescriptorImageInfo{
         .sampler     = VK_NULL_HANDLE,
         .imageView   = this->canvas.oit.accumulator.view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      auto image_info_oit_reveal = VkDescriptorImageInfo{
         .sampler     = VK_NULL_HANDLE,
         .imageView   = this->canvas.oit.reveal.view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      //
      auto& list = this->swap_chain.frames_in_flight;
      for (auto& frame : list) {
         auto global_state_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_data.handle,
            .offset = 0,
            .range  = sizeof(scene_global_state), // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto shad_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.light_shadow_data.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto bounds_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_bounds.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto landscape_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_landscapes.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto rlsp_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_lights.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto rmsp_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_meshes.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         //
         auto frustum_main_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_frustums.main.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto frustum_sun_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_frustums.sun.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         //
         auto light_position_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.active_caster_positions.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto mesh_bounds_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.mesh_bounds.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         //
         auto culling_mesh_params_main_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.main.params.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto culling_mesh_indices_main_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.main.mesh_indices.gpu.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto culling_mesh_params_sun_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.sun_shadows.params.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto culling_mesh_indices_sun_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.sun_shadows.mesh_indices.gpu.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         //
         std::array<VkDescriptorBufferInfo, config::max_active_shadow_casters> culling_caster_params_buffer_info = ([&frame]() {
            std::array<VkDescriptorBufferInfo, config::max_active_shadow_casters> out;
            for (size_t i = 0; i < out.size(); ++i) {
               out[i] = {
                  .buffer = frame.indirect_draw_commands.shadow_casters[i].params.handle,
                  .offset = 0,
                  .range  = VK_WHOLE_SIZE,
               };
            }
            return out;
         })();
         std::array<VkDescriptorBufferInfo, config::max_active_shadow_casters> culling_caster_mesh_indices_buffer_info = ([&frame]() {
            std::array<VkDescriptorBufferInfo, config::max_active_shadow_casters> out;
            for (size_t i = 0; i < out.size(); ++i) {
               out[i] = {
                  .buffer = frame.indirect_draw_commands.shadow_casters[i].mesh_indices.gpu.handle,
                  .offset = 0,
                  .range  = VK_WHOLE_SIZE,
               };
            }
            return out;
         })();
         //
         std::array<VkDescriptorImageInfo, shadow_caster_count> light_shadow_info;
         for (size_t i = 0; i < shadow_caster_count; ++i) {
            auto& entry = this->canvas.light_shadows.resources[i];
            light_shadow_info[i] = VkDescriptorImageInfo{
               .sampler     = this->canvas.light_shadows.sampler,
               .imageView   = entry.cubemap.view,
               .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
         }
         //
         auto descriptor_writes = cobb::array_concat(
            //
            // Scene state:
            //
            ([&frame, &global_state_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.scene_state;
               auto out = std::array{
                  VkWriteDescriptorSet{ // uniform buffer object
                     .dstSet           = descriptor_set,
                     .dstBinding       = 0,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &global_state_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               return out;
            })(),
            //
            // Shadow maps:
            //
            ([&frame, &image_info_sun_shadow, &light_shadow_info]() {
               auto descriptor_set = frame.descriptor_sets.shadow_maps;
               auto out = std::array{
                  VkWriteDescriptorSet{ // sun shadow map
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = (uint32_t)1,
                     .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                     .pImageInfo      = &image_info_sun_shadow,
                  },
                  VkWriteDescriptorSet{ // light shadow maps
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = (uint32_t)light_shadow_info.size(),
                     .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                     .pImageInfo      = light_shadow_info.data(),
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // Shadow caster map render:
            //
            ([&frame, &shad_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.shadow_caster_map_render;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object: mat4[shadow_caster_count][6]
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &shad_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // All textures:
            //
            ([&frame, &sampler_info, &texture_infos]() {
               auto descriptor_set = frame.descriptor_sets.all_textures;
               auto out = std::array{
                  VkWriteDescriptorSet{ // texture sampler
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = 1,
                     .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                     .pImageInfo      = &sampler_info,
                  },
                  VkWriteDescriptorSet{ // texture array
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = (uint32_t)texture_infos.size(),
                     .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                     .pImageInfo      = texture_infos.data(),
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // All bounds:
            //
            ([&frame, &bounds_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.all_bounds;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &bounds_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // All landscapes:
            //
            ([&frame, &landscape_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.all_landscapes;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &landscape_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // All lights:
            //
            ([&frame, &rlsp_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.all_lights;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object: rendered_light::shader_parameters[]
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &rlsp_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // All meshes:
            //
            ([&frame, &rmsp_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.all_meshes;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object: rendered_mesh::shader_parameters[]
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &rmsp_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // Compute: frustum culling:
            //
            ([&frame, &frustum_main_buffer_info, &mesh_bounds_buffer_info, &culling_mesh_indices_main_buffer_info, &culling_mesh_params_main_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.sharing_sets.compute_frustum_culling_main;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object: frustum plane normals
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &frustum_main_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: rendered_mesh::cull_data[]
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &mesh_bounds_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: object index buffer
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &culling_mesh_indices_main_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: object index buffer
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &culling_mesh_params_main_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // Compute: frustum culling: sun:
            //
            ([&frame, &frustum_main_buffer_info, &mesh_bounds_buffer_info, &culling_mesh_indices_sun_buffer_info, &culling_mesh_params_sun_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.sharing_sets.compute_frustum_culling_sun;
               auto out = std::array{
                  VkWriteDescriptorSet{ // storage buffer object: frustum plane normals
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &frustum_main_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: rendered_mesh::cull_data[]
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &mesh_bounds_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: object index buffer
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &culling_mesh_indices_sun_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: object index buffer
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &culling_mesh_params_sun_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })(),
            //
            // Compute: shadow caster culling
            //
            ([&frame, &mesh_bounds_buffer_info, &light_position_buffer_info, &culling_caster_mesh_indices_buffer_info, &culling_caster_params_buffer_info]() {
               using caster_write_list_t = std::array<VkWriteDescriptorSet, 4>;
               //
               std::array<caster_write_list_t, config::max_active_shadow_casters> casters;
               for (size_t i = 0; i < casters.size(); ++i) {
                  auto descriptor_set = frame.descriptor_sets.sharing_sets.compute_shadow_caster_culls[i];
                  casters[i] = std::array{
                     VkWriteDescriptorSet{ // storage buffer object: rendered_mesh::cull_data[]
                        .dstSet           = descriptor_set,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1,
                        .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo       = nullptr,
                        .pBufferInfo      = &mesh_bounds_buffer_info,
                        .pTexelBufferView = nullptr,
                     },
                     VkWriteDescriptorSet{ // storage buffer object: glm::vec4
                        .dstSet           = descriptor_set,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1, // this should be 1 because we are updating 1 buffer; that the buffer's data is used as an array on the shader side is irrelevant
                        .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo       = nullptr,
                        .pBufferInfo      = &light_position_buffer_info,
                        .pTexelBufferView = nullptr,
                     },
                     VkWriteDescriptorSet{ // storage buffer object: mesh indices array
                        .dstSet           = descriptor_set,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1,
                        .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo       = nullptr,
                        .pBufferInfo      = &culling_caster_mesh_indices_buffer_info[i],
                        .pTexelBufferView = nullptr,
                     },
                     VkWriteDescriptorSet{ // storage buffer object: draw params array
                        .dstSet           = descriptor_set,
                        .dstArrayElement  = 0,
                        .descriptorCount  = 1,
                        .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo       = nullptr,
                        .pBufferInfo      = &culling_caster_params_buffer_info[i],
                        .pTexelBufferView = nullptr,
                     },
                  };
                  for (size_t j = 0; j < casters[i].size(); ++j)
                     casters[i][j].dstBinding = j;
               }
               auto out = cobb::array_concat(casters[0], casters[1], casters[2], casters[3]);
               return out;
            })(),
            //
            // OIT composite pass:
            //
            ([&frame, &image_info_oit_accumulator, &image_info_oit_reveal]() {
               auto descriptor_set = frame.descriptor_sets.oit_compositing;
               auto out = std::array{
                  VkWriteDescriptorSet{
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = (uint32_t)1,
                     .descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                     .pImageInfo      = &image_info_oit_accumulator,
                  },
                  VkWriteDescriptorSet{
                     .dstSet          = descriptor_set,
                     .dstArrayElement = 0,
                     .descriptorCount = (uint32_t)1,
                     .descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                     .pImageInfo      = &image_info_oit_reveal,
                  },
               };
               for (size_t i = 0; i < out.size(); ++i)
                  out[i].dstBinding = i;
               return out;
            })()//,
         );
         for (auto& item : descriptor_writes)
            item.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
         //
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
         entry.handled_frames.set_all_up_to_date();
   }
   //
   void surface_renderer::_setup_render_passes() {
      this->render_passes_by_name.main->attachments[0].format = this->swap_chain.format;
      if (auto* rp = this->render_passes_by_name.main_oit) {
         rp->attachments[2].format = this->swap_chain.format;
      }
      this->render_passes_by_name.bounds->attachments[0].format = this->swap_chain.format;
      this->render_passes_by_name.ui->attachments[0].format = this->swap_chain.format;
      //
      // (Re)create the render passes within the GPU:
      //
      for(auto* rp : this->render_passes)
         if (rp)
            rp->setup();
      //
      this->set_debug_object_name(this->render_passes_by_name.main_shadow->handle, "Render Pass: Sun Shadows");
      this->set_debug_object_name(this->render_passes_by_name.main->handle, "Render Pass: Main");
      if (auto* rp = this->render_passes_by_name.main_oit) {
         this->set_debug_object_name(rp->handle, "Render Pass: Main OIT");
      }
      this->set_debug_object_name(this->render_passes_by_name.bounds->handle, "Render Pass: Bounds");
      this->set_debug_object_name(this->render_passes_by_name.ui->handle,   "Render Pass: UI");
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
         .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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
      if (auto result = vkCreateSwapchainKHR(this->logical_device, &create_info, nullptr, &sc.handle); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::surface_renderer::_setup_swap_chain_instance] Failed to create swap chain.");
      }
   }
   void surface_renderer::_setup_depth_buffer() {
      auto  extent = this->surface_extent;
      auto  format = this->find_depth_format();
      auto& db     = this->canvas.depth;
      db = owned_image_and_view(*this);
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
      db.transition_layout(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
      //
      this->set_debug_object_name(db.handle, "Depth Buffer Image");
      this->set_debug_object_name(db.view,   "Depth Buffer Image View");
   }
   void surface_renderer::_setup_color_buffer() {
      auto  extent = this->surface_extent;
      auto& db     = this->canvas.color;
      db = owned_image_and_view(*this);
      db.create_image(
         {
            .extent = {
               .width  = extent.width,
               .height = extent.height,
            },
            .format = this->swap_chain.format,
            .usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
         }, 
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      db.create_basic_view(this->swap_chain.format, VK_IMAGE_ASPECT_COLOR_BIT);
      db.transition_layout(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
      //
      this->set_debug_object_name(db.handle, "Color Buffer Image");
      this->set_debug_object_name(db.view,   "Color Buffer Image View");
   }
   void surface_renderer::_setup_sun_shadow_buffer() {
      auto  format = this->find_depth_format();
      auto& image  = this->canvas.sun_shadow.map;
      image = owned_image_and_view(*this);
      image.create_image(
         {
            .extent = {
               .width  = config::sun_shadow_map_resolution_x,
               .height = config::sun_shadow_map_resolution_y,
               .depth  = 1,
            },
            .format = format,
            .usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         }, 
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
      );
      image.create_basic_view(format, VK_IMAGE_ASPECT_DEPTH_BIT);
      image.transition_layout(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
      //
      this->set_debug_object_name(image.handle, "Sun Shadow Buffer Image");
      this->set_debug_object_name(image.view,   "Sun Shadow Buffer Image View");
      //
      // Sampler:
      //
      const auto& support = this->device_info->support;
      auto& sampler = this->canvas.sun_shadow.sampler;
      auto  sampler_info = VkSamplerCreateInfo{
         .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter        = VK_FILTER_NEAREST,
         .minFilter        = VK_FILTER_NEAREST,
         .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
         .addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
         .mipLodBias       = 0.0,
         .anisotropyEnable = support.max_anisotropic_filtering > 0.0 ? VK_TRUE : VK_FALSE,
         .maxAnisotropy    = std::min(8.0F, support.max_anisotropic_filtering),
         .compareEnable    = VK_FALSE,
         .compareOp        = VK_COMPARE_OP_LESS,
         .minLod           = 0.0,
         .maxLod           = 1.0,
         .borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
         .unnormalizedCoordinates = VK_FALSE,
      };
      if (auto result = vkCreateSampler(this->logical_device, &sampler_info, nullptr, &sampler); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::surface_renderer::_setup_raw_pixel_texture_sampler] Failed to create the raw pixel texture sampler.");
      }
      this->set_debug_object_name(sampler, "Sun Shadow Texture Sampler");
   }
   void surface_renderer::_setup_light_shadow_resources() {
      auto& res_list = this->canvas.light_shadows.resources;
      //
      const auto& support = this->device_info->support;
      const auto  aspect  = VK_IMAGE_ASPECT_COLOR_BIT;
      const auto  format  = VK_FORMAT_R32_SFLOAT;
      //
      {  // Cubemap sampler
         const auto& support = this->device_info->support;
         //
         auto& sampler = this->canvas.light_shadows.sampler;
         auto  sampler_info = VkSamplerCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter        = VK_FILTER_NEAREST,
            .minFilter        = VK_FILTER_NEAREST,
            .mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .mipLodBias       = 0.0,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy    = 1.0,
            .compareEnable    = VK_FALSE,
            .compareOp        = VK_COMPARE_OP_NEVER,
            .minLod           = 0.0,
            .maxLod           = 1.0,
            .borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
         };
         if (auto result = vkCreateSampler(this->logical_device, &sampler_info, nullptr, &sampler); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::surface_renderer::_setup_light_shadow_resources] Failed to create the raw pixel texture sampler.");
         }
         this->set_debug_object_name(sampler, "Light Shadow Texture Sampler");
      }
      //
      const vulkanDK::image_metadata metadata = {
         .extent = {
            .width  = config::light_shadow_map_resolution_x,
            .height = config::light_shadow_map_resolution_y,
            .depth  = 1,
         },
         .format      = format,
         .is_cubemap  = true,
         .layer_count = 6,
         .usage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      };
      //
      for (auto& entry : res_list) {
         auto& image = entry.cubemap;
         //
         image = owned_image_and_view(*this);
         image.create_image(metadata, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
         image.create_basic_view(format, aspect);
         image.transition_layout(aspect, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
         //
         this->set_debug_object_name(image.handle, "Light Shadow Buffer Image");
         this->set_debug_object_name(image.view,   "Light Shadow Buffer Image View");
      }
      //
      // Framebuffer:
      //
      auto attachments = std::array{
         res_list[0].cubemap.view,
         res_list[1].cubemap.view,
         res_list[2].cubemap.view,
         res_list[3].cubemap.view,
      };
      auto framebuffer_info = VkFramebufferCreateInfo{
         .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
         .pNext           = nullptr,
         .flags           = 0,
         .renderPass      = this->render_passes_by_name.main_shadow_placed->handle,
         .attachmentCount = attachments.size(),
         .pAttachments    = attachments.data(),
         .width           = config::light_shadow_map_resolution_x,
         .height          = config::light_shadow_map_resolution_y,
         .layers          = 1, // for multiview, you must use 1 rather than the actual layer count
      };
      if (auto result = vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &this->canvas.light_shadows.framebuffer); result != VK_SUCCESS) {
         throw result_exception(result, "[vulkanDK::surface_renderer::_setup_light_shadow_resources] Failed to create a framebuffer (light shadows).");
      }
      this->set_debug_object_name(this->canvas.light_shadows.framebuffer, "Framebuffer (Light Shadows)");
   }
   void surface_renderer::_setup_oit_images() {
      constexpr auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
      //
      auto extent = this->surface_extent;
      {
         constexpr auto format = format_for_oit_accumulator;
         auto& image = this->canvas.oit.accumulator;
         image = owned_image_and_view(*this);
         image.create_image(
            {
               .extent = {
                  .width  = extent.width,
                  .height = extent.height,
               },
               .format = format,
               .usage  = usage,
            }, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
         );
         image.create_basic_view(format, VK_IMAGE_ASPECT_COLOR_BIT);
         image.transition_layout(
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
         );
         //
         this->set_debug_object_name(image.handle, "OIT Accumulator Image");
         this->set_debug_object_name(image.view,   "OIT Accumulator Image View");
      }
      {
         constexpr auto format = format_for_oit_reveal;
         auto& image = this->canvas.oit.reveal;
         image = owned_image_and_view(*this);
         image.create_image(
            {
               .extent = {
                  .width  = extent.width,
                  .height = extent.height,
               },
               .format = VK_FORMAT_R16_SFLOAT,
               .usage  = usage,
            }, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
         );
         image.create_basic_view(format, VK_IMAGE_ASPECT_COLOR_BIT);
         image.transition_layout(
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
         );
         //
         this->set_debug_object_name(image.handle, "OIT Reveal Image");
         this->set_debug_object_name(image.view,   "OIT Reveal Image View");
      }
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
      //
      // Get the image handles and set up image state (descriptor sets, views, etc.):
      //
      std::vector<VkImage> image_handles(image_count);
      vkGetSwapchainImagesKHR(this->logical_device, sc.handle, &image_count, image_handles.data());
      for (size_t i = 0; i < image_count; ++i) {
         sc.images[i].setup(*this, i);
         //
         auto& sci = sc.images[i].image;
         sci = image_and_view(*this);
         sci.handle   = image_handles[i];
         sci.metadata = image_metadata{
            .dimensions   = VkImageType::VK_IMAGE_TYPE_2D,
            .extent       = {
               .width  = this->surface_extent.width,
               .height = this->surface_extent.height,
               .depth  = 1
            },
            .format       = sc.format,
            .is_cubemap   = false,
            .layer_count  = 1,
            .mipmap_count = 1,
            .samples      = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
            .sharing      = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
            .tiling       = VkImageTiling::VK_IMAGE_TILING_OPTIMAL,
            .usage        = 0,
         };
         sc.images[i].image.create_basic_view(sc.format, VK_IMAGE_ASPECT_COLOR_BIT);
         //
         this->set_debug_object_name(sci.handle, QString("Swap Chain Image %1").arg(i).toStdString());
         this->set_debug_object_name(sci.view,   QString("Swap Chain Image View %1").arg(i).toStdString());
         //
         sc.images[i].record_final_blit_command();
      }
   }
   void surface_renderer::_setup_frames_in_flight() {
      auto& list = this->swap_chain.frames_in_flight;
      for (size_t i = 0; i < list.size(); ++i) {
         auto& item = list[i];
         item.setup(*this, i);
      }
   }
   void surface_renderer::_setup_framebuffers() {
      auto extent = this->surface_extent;
      {  // Main framebuffer
         auto attachments      = std::array{ this->canvas.color.view, this->canvas.depth.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = this->render_passes_by_name.main->handle,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1,
         };
         if (auto result = vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &this->canvas.main_framebuffer); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::surface_renderer::_setup_framebuffers] Failed to create the main framebuffer.");
         }
         this->set_debug_object_name(this->canvas.main_framebuffer, "Framebuffer (Main)");
      }
      if (this->canvas.sun_shadow.framebuffer == VK_NULL_HANDLE) { // Sun shadow framebuffer
         auto attachments      = std::array{ this->canvas.sun_shadow.map.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = this->render_passes_by_name.main_shadow->handle,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = config::sun_shadow_map_resolution_x,
            .height          = config::sun_shadow_map_resolution_y,
            .layers          = 1,
         };
         if (auto result = vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &this->canvas.sun_shadow.framebuffer); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::surface_renderer::_setup_framebuffers] Failed to create a framebuffer (sun shadows).");
         }
         this->set_debug_object_name(this->canvas.sun_shadow.framebuffer, "Framebuffer (Sun Shadows)");
      }
      if (this->can_do_alpha()) { // OIT framebuffer
         //
         // If the device doesn't support the features we need for OIT, then we don't even 
         // define the render pass, so we also shouldn't create framebuffers that need it.
         //
         assert(this->render_passes_by_name.main_oit != nullptr);
         auto attachments      = std::array{ this->canvas.oit.accumulator.view, this->canvas.oit.reveal.view, this->canvas.color.view, this->canvas.depth.view };
         auto framebuffer_info = VkFramebufferCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = nullptr,
            .flags           = 0,
            .renderPass      = this->render_passes_by_name.main_oit->handle,
            .attachmentCount = attachments.size(),
            .pAttachments    = attachments.data(),
            .width           = extent.width,
            .height          = extent.height,
            .layers          = 1,
         };
         if (auto result = vkCreateFramebuffer(this->logical_device, &framebuffer_info, nullptr, &this->canvas.oit.framebuffer); result != VK_SUCCESS) {
            throw result_exception(result, "[vulkanDK::surface_renderer::_setup_framebuffers] Failed to create a framebuffer (OIT).");
         }
         this->set_debug_object_name(this->canvas.oit.framebuffer, "Framebuffer (OIT)");
      }
   }
   //
   void surface_renderer::_setup_descriptor_pool() {
      auto& dsl = this->descriptor_set_layouts;
      this->setup_descriptor_pool(
         dsl.needed_pool_sizes(),
         dsl.total_set_count()
      );
   }

   void surface_renderer::teardown() {
      this->_on_renderer_teardown_imminent();
      if (this->logical_device == VK_NULL_HANDLE) {
         //
         // It must be the case that we weren't able to properly set up at all, or else 
         // some precondition to setup failed  and we're just blindly being called as a 
         // result of a destructor after not even trying to set up.
         //
         return;
      }
      //
      if (auto result = vkDeviceWaitIdle(this->logical_device); result != VK_SUCCESS) {
         throw result_exception(result, "[surface_renderer::teardown] Device-wait failed.");
      }
      //
      // Ensure all child objects belonging to the instance are destroyed.
      //
      this->scene.teardown(*this);
      this->null_texture.teardown();
      {
         auto& list = this->graphics_shaders;
         for (auto* e : list)
            delete e;
         list.clear();
      }
      {
         auto& list = this->compute_shaders;
         for (auto* e : list)
            delete e;
         list.clear();
      }
      {  // Swap chain
         auto& sc = this->swap_chain;
         //
         sc.images.clear();
         if (this->canvas.sun_shadow.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(this->logical_device, this->canvas.sun_shadow.sampler, nullptr);
            this->canvas.sun_shadow.sampler = VK_NULL_HANDLE;
         }
         if (auto& handle = this->canvas.main_framebuffer; handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, handle, nullptr);
            handle = VK_NULL_HANDLE;
         }
         if (auto& handle = this->canvas.sun_shadow.framebuffer; handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, handle, nullptr);
            handle = VK_NULL_HANDLE;
         }
         {
            auto& ls = this->canvas.light_shadows;
            if (ls.sampler != VK_NULL_HANDLE) {
               vkDestroySampler(this->logical_device, ls.sampler, nullptr);
               ls.sampler = VK_NULL_HANDLE;
            }
            if (auto& handle = ls.framebuffer; handle != VK_NULL_HANDLE) {
               vkDestroyFramebuffer(this->logical_device, handle, nullptr);
               handle = VK_NULL_HANDLE;
            }
            for (auto& item : ls.resources) {
               item.light_index = scene::index_of_none;
               item.cubemap.teardown();
            }
         }
         if (auto& handle = this->canvas.oit.framebuffer; handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, handle, nullptr);
            handle = VK_NULL_HANDLE;
         }
         this->canvas.color.teardown();
         this->canvas.depth.teardown();
         this->canvas.oit.accumulator.teardown();
         this->canvas.oit.reveal.teardown();
         this->canvas.sun_shadow.map.teardown();
         vkDestroySwapchainKHR(this->logical_device, sc.handle, nullptr);
         sc.handle = VK_NULL_HANDLE;
         //
         for (auto& fif : sc.frames_in_flight)
            fif.teardown();
      }
      this->_teardown_raw_pixel_texture_sampler();
      for (auto& e : this->render_passes_by_name._list)
         e = nullptr; // deletion will be handled in abstract_renderer::teardown
      //
      if constexpr (use_vma_library) {
         vmaDestroyAllocator(this->allocator);
      }
      if (this->one_time_commands_fence != VK_NULL_HANDLE) {
         vkDestroyFence(this->logical_device, this->one_time_commands_fence, nullptr);
         this->one_time_commands_fence = VK_NULL_HANDLE;
      }
      //
      for (auto& q : this->queues.list)
         q.teardown_command_pools(this->logical_device);
      abstract_renderer::start_teardown();
      this->descriptor_set_layouts.teardown_all();
      abstract_renderer::end_teardown(); // tears down the logical device
      //
      this->_on_renderer_teardown_complete();
   }

   void surface_renderer::handle_resize() {
      auto  device = this->logical_device;
      auto& sc     = this->swap_chain;
      //
      if (auto result = vkDeviceWaitIdle(device); result != VK_SUCCESS) {
         throw result_exception(result, "[surface_renderer::handle_resize] Device-wait failed.");
      }
      //
      VkFormat sc_format = sc.format;
      size_t   sc_count  = sc.images.size();
      {  // Tear down swap chain state
         for (auto& s : this->graphics_shaders)
            s->pre_resize();
         for (auto& image : sc.images)
            image.teardown();
         for (auto& fif : sc.frames_in_flight)
            fif.pre_resize();
         if (auto& handle = this->canvas.main_framebuffer; handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, handle, nullptr);
            handle = VK_NULL_HANDLE;
         }
         //
         // Don't destroy the sun shadow framebuffer; we don't need to resize it with the surface.
         //
         if (auto& handle = this->canvas.oit.framebuffer; handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, handle, nullptr);
            handle = VK_NULL_HANDLE;
         }
         this->canvas.depth.teardown();
         this->canvas.color.teardown();
         vkDestroySwapchainKHR(this->logical_device, sc.handle, nullptr);
         sc.handle = VK_NULL_HANDLE;
      }
      {
         auto& fb = this->canvas.oit.framebuffer;
         if (fb != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(this->logical_device, fb, nullptr);
            fb = VK_NULL_HANDLE;
         }
         this->canvas.oit.accumulator.teardown();
         this->canvas.oit.reveal.teardown();
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
            if (auto* rp = this->render_passes_by_name.main_oit) {
               rp->teardown();
               rp->attachments[2].format = sc.format;
               rp->setup();
            }
            if (auto* rp = this->render_passes_by_name.bounds) {
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
         for (auto& s : this->graphics_shaders)
            s->post_resize(this->surface_extent);
         this->_setup_depth_buffer(); // requires surface extent
         this->_setup_color_buffer(); // requires surface extent
         this->_setup_oit_images();   // requires surface extent
         for (auto& fif : this->swap_chain.frames_in_flight)
            fif.post_resize();
         this->_initialize_descriptor_sets(); // need to send the color and depth images back to the FIFs
         this->_setup_swap_chain_images();
         this->_setup_framebuffers(); // requires surface extent
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
      for (auto& item : this->scene.meshes)
         item.handled_frames.set_all_out_of_date();
      for (auto& item : this->scene.lights)
         item.handled_frames.set_all_out_of_date();
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
      auto& time_prior = this->state.last_frame_at;
      if (time_prior.time_since_epoch() == timestamp_t::duration::zero()) {
         time_prior = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      }
      //
      // If this frame-in-flight is still being used to render and present another swap 
      // chain image, wait for it to finish. We'll also advance the current frame counter 
      // here.
      //
      auto& fif = sc.frames_in_flight[sc.current_frame];
      sc.current_frame = (sc.current_frame + 1) % sc.frames_in_flight.size();
      fif.fences.wait_on_all(this->logical_device);
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
            throw result_exception(result, "[vulkanDK::surface_renderer::draw_next_frame] Failed to acquire swap chain image!");
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
            throw result_exception(result, "[vulkanDK::surface_renderer::draw_next_frame] Failed to present swap chain image.");
      }
      //
      // Post-draw behavior:
      //
      auto time_after = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      {
         this->state.last_frame_time = std::chrono::duration<double, std::chrono::seconds::period>(time_after - time_prior).count();
         this->state.last_frame_at   = time_after;
         //
         this->state.fps.next_delta(this->state.last_frame_time);
      }
      this->_execute_pending_scene_deletions();
   }


   command_buffer surface_renderer::_begin_one_time_commands(queue& q) {
      //
      // TODO: This is a useful helper function, but you'll actually get higher throughput if you 
      // reuse a single command buffer instead of spawning several temporary buffers; you'd want 
      // to have a function to create that single reusable buffer, and a "flush" function to 
      // execute whatever commands have been recorded so far.
      // 
      // See the end of: https://vulkan-tutorial.com/en/Texture_mapping/Images#page_Transition-barrier-masks
      //
      auto scratch = command_buffer::create_transient(*this, q);
      //
      auto begin_info = VkCommandBufferBeginInfo{
         .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      };
      vkBeginCommandBuffer(scratch.handle, &begin_info);
      //
      return scratch;
   }
   void surface_renderer::_end_one_time_commands(command_buffer& scratch, queue& q) {
      vkEndCommandBuffer(scratch.handle);
      //
      auto submit_info = VkSubmitInfo{
         .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1,
         .pCommandBuffers    = &scratch.handle,
      };
      vkResetFences(this->logical_device, 1, &this->one_time_commands_fence);
      if (auto result = vkQueueSubmit(q.handle, 1, &submit_info, this->one_time_commands_fence); result != VK_SUCCESS) {
         throw result_exception(result, "[surface_renderer::_end_one_time_commands] Submission failed.");
      }
      if (auto result = vkWaitForFences(this->logical_device, 1, &this->one_time_commands_fence, VK_FALSE, UINT64_MAX); result != VK_SUCCESS) {
         throw result_exception(result, "[surface_renderer::_end_one_time_commands] Wait-for-completion failed.");
      }
   }

   bool surface_renderer::can_do_alpha() const {
      return this->device_info->support.independent_blending;
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
         throw exception("[vulkanDK::surface_renderer::find_depth_format] No format.");
      }
      return fmt;
   }
   bool surface_renderer::needs_null_texture() const {
      return this->device_info->support.descriptor_bindings.null_handles == false;
   }

   buffer surface_renderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
      return buffer::create(*this, size, usage, properties);
   }
   void surface_renderer::set_debug_object_name(uint64_t handle, VkObjectType type, const std::string& name) {
      if constexpr (!config::enable_validation_layers) {
         return;
      }
      if (!this->api_functions.vkSetDebugUtilsObjectNameEXT) {
         if constexpr (debug_object_names_fallback_to_log) {
            //qDebug("[surface_renderer::set_debug_object_name] Extension unavailable; name for handle %016jX is %s.", (std::uintmax_t)handle, name.c_str());
            // getting crashes when using %016jX; just %016X causes int argument truncation; just stringify it manually:
            QString text = "[surface_renderer::set_debug_object_name] Extension unavailable; name for handle 0x";
            text += QString::number(handle, 16).leftJustified(16, '0');
            text += " is ";
            text += name.c_str();
            text += ".";
            qDebug("%s", qUtf8Printable(text));
         }
         return;
      }
      auto info = VkDebugUtilsObjectNameInfoEXT{
         .sType        = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
         .pNext        = nullptr,
         .objectType   = type,
         .objectHandle = handle,
         .pObjectName  = name.c_str(),
      };
      (this->api_functions.vkSetDebugUtilsObjectNameEXT)(this->logical_device, &info);
   }

   graphics_shader* surface_renderer::get_graphics_shader(cobb::eight_cc id) const {
      for (auto* s : this->graphics_shaders)
         if (s->id == id)
            return s;
      return nullptr;
   }
   graphics_shader* surface_renderer::create_graphics_shader(cobb::eight_cc id) {
      assert(!this->get_graphics_shader(id));
      auto* s = new graphics_shader(*this);
      s->id = id;
      this->graphics_shaders.push_back(s);
      return s;
   }

   compute_shader* surface_renderer::create_compute_shader(cobb::eight_cc id) {
      assert(!this->get_compute_shader(id));
      auto* s = new compute_shader(*this);
      s->id = id;
      this->compute_shaders.push_back(s);
      return s;
   }
   compute_shader* surface_renderer::get_compute_shader(cobb::eight_cc id) const {
      for (auto* s : this->compute_shaders)
         if (s->id == id)
            return s;
      return nullptr;
   }

   #pragma region scene
   size_t surface_renderer::add_texture(const QString& texture_path) {
      constexpr size_t fail = scene::index_of_none;
      //
      auto& list = this->scene.textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& prior = list[i];
         if (prior.path == texture_path) {
            if constexpr (debug_log_scene_object_lifetimes) {
               qDebug("[vulkanDK::scene_renderer::add_texture] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(texture_path));
            }
            if (!prior.active()) {
               if (!prior.pending_delete() || prior.content.handle == VK_NULL_HANDLE) {
                  //
                  // Texture slot matches our path, but the slot isn't active or pending deletion, 
                  // or its texture content is gone. This shouldn't happen.
                  //
                  #if _DEBUG
                     __debugbreak();
                  #endif
                  continue;
               }
               //
               // Rescue the texture from deletion.
               //
               prior.life_state = scene_frame_item_state::active;
               prior.handled_frames.set_all_out_of_date();
               --this->scene.pending_deletions.textures;
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
         if (texture_index == scene::index_of_none) {
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
         target.content = owned_image_and_view(*this);
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
               img.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
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
      if (texture_index == scene::index_of_none) {
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
      target.content = owned_image_and_view(*this);
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
      target.content.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
      target.content.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
      //
      target.handled_frames.set_all_out_of_date();
      return texture_index;
   }
   size_t surface_renderer::add_dds_texture(QString texture_path) {
      constexpr size_t fail = scene::index_of_none;
      //
      if (!texture_path.endsWith(".dds", Qt::CaseInsensitive))
         return fail;
      texture_path = QDir::cleanPath(texture_path).toLower();
      //
      auto& list = this->scene.textures;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& prior = list[i];
         if (prior.path == texture_path) {
            if constexpr (debug_log_scene_object_lifetimes) {
               qDebug("[vulkanDK::scene_renderer::add_dds_texture] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(texture_path));
            }
            if (!prior.active()) {
               if (!prior.pending_delete() || prior.content.handle == VK_NULL_HANDLE) {
                  //
                  // Texture slot matches our path, but the slot isn't active or pending deletion, 
                  // or its texture content is gone. This shouldn't happen.
                  //
                  #if _DEBUG
                     __debugbreak();
                  #endif
                  continue;
               }
               //
               // Rescue the texture from deletion.
               //
               prior.life_state = scene_frame_item_state::active;
               prior.handled_frames.set_all_out_of_date();
               --this->scene.pending_deletions.textures;
            }
            return i;
         }
      }
      //
      std::unique_ptr<dovah::bsa_archived_file> file;
      {
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
      if (texture_index == scene::index_of_none) {
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
      target.content = owned_image_and_view(*this);
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
            img.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
         }
         img.create_basic_view(img.metadata.format, VK_IMAGE_ASPECT_COLOR_BIT);
         //
         #if _DEBUG
            this->set_debug_object_name(img.handle, QString("2D Image <Tex %1> <%2>").arg(texture_index).arg(target.path).toStdString());
            this->set_debug_object_name(img.view,   QString("Image View <Tex %1> <%2>").arg(texture_index).arg(target.path).toStdString());
         #endif
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
      if (texture_index == scene::index_of_none) {
         qDebug("Cannot add new rendered object: failed to add its texture.");
         return;
      }
      auto&  texture_item = this->scene.textures[texture_index];
      size_t object_index = this->scene.insert_new_mesh();
      if (object_index == scene::index_of_none) {
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
               .color  = { 1, 1, 1, 1 },
               .uv     = { 1, 0 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { 0.5f, -hfwc, 0.0 },
               .color  = { 1, 1, 1, 1 },
               .uv     = { 0, 0 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { 0.5f, hfwc, 0.0 },
               .color  = { 1, 1, 1, 1 },
               .uv     = { 0, 1 },
               .normal    = { 0, 0, 1 },
               .tangent   = { 1, 0, 0 },
               .bitangent = { 0, 1, 0 },
            },
            vertex{
               .pos    = { -0.5f, hfwc, 0.0 },
               .color  = { 1, 1, 1, 1 },
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
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_meshes_added_or_removed();
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
      if (item.owning_nif) {
         item.owning_nif->sever_connection_to(rendered_mesh_handle(*this, i));
         item.owning_nif = nullptr;
      }
      {
         auto& list = this->scene.textures;
         for (auto& ti : item.texture_indices.list) {
            if (ti < 0)
               continue;
            assert(ti < list.size());
            if (ti < list.size()) {
               auto& tex = list[ti];
               if (this->scene.texture_dec_ref({}, tex)) {
                  if constexpr (debug_log_scene_object_lifetimes) {
                     qDebug("[vulkanDK::scene_renderer::remove_mesh] Mesh %u used texture %u which is now unused; marking the texture for delete.", i, ti);
                  }
               }
            }
            ti = -1;
         }
      }
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_meshes_added_or_removed();
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
   void surface_renderer::remove_light(size_t i) {
      auto& list = this->scene.lights;
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_light] Marking scene light %u for delete.", i);
      }
      ++this->scene.pending_deletions.lights;
      auto& item = list[i];
      if (item.can_cast_shadows()) {
         this->scene.mark_light_shadows_dirty();
      }
      item.mark_for_delete();
   }
   void surface_renderer::remove_last_light() {
      auto& list = this->scene.lights;
      auto  size = list.size();
      if (size == 0)
         return;
      for (size_t i = size - 1; i >= 0; --i) {
         auto& item = list[i];
         if (!item.active())
            continue;
         this->remove_light(i);
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
   void surface_renderer::surface_position_to_world_ray(int x, int y, glm::vec3& eye_position, glm::vec3& eye_direction) const {
      auto viewport = glm::vec4( 0, 0, this->surface_extent.width, this->surface_extent.height );
      //
      eye_position = glm::unProjectZO(
         glm::vec3{ x, y, config::use_inverted_depth ? 1.0 : 0.0 },
         this->scene.global_state.view,
         this->scene.global_state.proj,
         viewport
      );
      auto eye_endpoint = glm::unProjectZO(
         glm::vec3{ x, y, config::use_inverted_depth ? 0.5 : 1.0 }, // don't use 0.0 for inverted far, because when we invert depth, we also push the far plane to infinity
         this->scene.global_state.view,
         this->scene.global_state.proj,
         viewport
      );
      eye_direction = glm::normalize(eye_endpoint - eye_position);
   }
   rendered_mesh_handle surface_renderer::rendered_mesh_at(int x, int y) {
      glm::vec3 eye_position;
      glm::vec3 eye_direction;
      this->surface_position_to_world_ray(x, y, eye_position, eye_direction);
      //
      size_t nearest  = -1;
      float  distance = std::numeric_limits<float>::max();
      //
      const auto& list = this->scene.meshes;
      for (size_t i = 0; i < list.size(); ++i) {
         const auto& mesh = list[i];
         //
         float hit_distance;
         if (!mesh.ray_intersects(eye_position, eye_direction, hit_distance))
            continue;
         if (hit_distance < distance) {
            distance = hit_distance;
            nearest = i;
         }
      }
      if (nearest == -1)
         return {};
      //
      // Double-check that objects of other types (e.g. landscapes) aren't in front of the 
      // hit mesh.
      //
      for (const auto& item : this->scene.landscapes) {
         float hit_distance;
         if (!item.ray_intersects(eye_position, eye_direction, hit_distance))
            continue;
         if (hit_distance < distance)
            //
            // There's a landscape in front of the nearest rendered mesh. Return a fail 
            // result.
            //
            return {};
      }
      //
      // If we reach this point, then there's nothing else obstructing the hit mesh.
      //
      return rendered_mesh_handle(*this, nearest);
   }

   rendered_bounds_handle surface_renderer::add_bounds(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform) {
      size_t index = this->scene.insert_new_bound();
      if (index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_bounds] Cannot add new rendered_bounds; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_bounds] Creating new bound at index %u.", index);
      }
      auto& item = this->scene.bounds[index];
      item.life_state = scene_frame_item_state::active;
      item.handled_frames.set_all_out_of_date();
      item.set_shader_params(min, max, pivot_transform);
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_bounds_added_or_removed();
      //
      return rendered_bounds_handle(*this, index);
   }
   void surface_renderer::remove_bounds(size_t i) {
      auto& list = this->scene.bounds;
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_bounds] Marking scene bounds %u for delete.", i);
      }
      ++this->scene.pending_deletions.bounds;
      auto& item = list[i];
      item.mark_for_delete();
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_bounds_added_or_removed();
   }

   rendered_landscape_handle surface_renderer::add_landscape(const glm::vec3& position) {
      size_t index = this->scene.insert_new_landscape();
      if (index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_landscape] Cannot add new rendered_landscape; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_landscape] Creating new landscape at index %u.", index);
      }
      auto& item = this->scene.landscapes[index];
      item.life_state = scene_frame_item_state::active;
      item.handled_frames.set_all_out_of_date();
      item.set_position(position);
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_landscape_added_or_removed();
      //
      return rendered_landscape_handle(*this, index);
   }
   rendered_landscape_handle surface_renderer::add_landscape(const glm::vec3& position, const dovah::loaded_forms::Landscape& land) {
      auto handle = this->add_landscape(position);
      if (handle.empty())
         return handle;
      //
      auto _load_textures = [this](int32_t& diffuse, int32_t& normal, dovah::form_stub& ltex) {
         auto  load = ltex.load().ptr_cast<dovah::loaded_forms::LandTexture>();
         if (!load)
            return;
         auto* txst = load->texture_set.get_form_stub();
         if (!txst)
            return;
         auto  txld = txst->load().ptr_cast<dovah::loaded_forms::TextureSet>();
         if (!txld)
            return;
         //
         diffuse = this->add_dds_texture(QString("textures/") + txld->textures.diffuse.c_str());
         normal  = this->add_dds_texture(QString("textures/") + txld->textures.normal.c_str());
      };
      //
      auto& sp = handle->shader_params;
      for (size_t i = 0; i < 4; ++i) {
         sp.diffuse_base[i] = -1;
         sp.normals_base[i] = -1;
         for (size_t j = 0; j < rendered_landscape::max_usable_layers_per_quad; ++j) {
            sp.diffuse_blends_by_quad[i][j] = -1;
            sp.normals_blends_by_quad[i][j] = -1;
         }
         //
         auto* ltex = land.default_quad_textures[i].get_form_stub();
         if (!ltex)
            continue;
         _load_textures(sp.diffuse_base[i], sp.normals_base[i], *ltex);
         //
         auto& blends = land.alpha_layers_by_quad[i];
         for (auto& blend : blends) {
            if (blend.layer < 0 || blend.layer >= rendered_landscape::max_usable_layers_per_quad)
               continue;
            auto* ltex = blend.texture.get_form_stub();
            if (!ltex)
               continue;
            _load_textures(sp.diffuse_blends_by_quad[i][blend.layer], sp.normals_blends_by_quad[i][blend.layer], *ltex);
         }
      }
      //
      handle->import_vertex_data_from_form(land);
      {
         this->_wait_on_all_frames_in_flight(); // ensure that we don't write to the coalesced landscape buffer while it is in use // TODO: double-buffering to work around this?
         this->scene.update_single_landscape(*this, handle.list_index({}));
      }
      return handle;
   }
   void surface_renderer::remove_landscape(size_t i) {
      auto& list = this->scene.landscapes;
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_landscape] Marking scene landscape %u for delete.", i);
      }
      auto& item = list[i];
      item.mark_for_delete();
      {
         auto _dec_tex_ref = [this, i](loaded_texture& tex, size_t ti) {
            if (this->scene.texture_dec_ref({}, tex)) {
               if constexpr (debug_log_scene_object_lifetimes) {
                  qDebug("[vulkanDK::scene_renderer::remove_landscape] Landscape %u used texture %u which is now unused; marking the texture for delete.", i, ti);
               }
            }
         };
         //
         auto& list = this->scene.textures;
         for (auto& ti : item.shader_params.diffuse_base) {
            if (ti < 0)
               continue;
            assert(ti < list.size());
            if (ti < list.size())
               _dec_tex_ref(list[ti], ti);
            ti = -1;
         }
         for (auto& ti : item.shader_params.diffuse_blends) {
            if (ti < 0)
               continue;
            assert(ti < list.size());
            if (ti < list.size())
               _dec_tex_ref(list[ti], ti);
            ti = -1;
         }
         for (auto& ti : item.shader_params.normals_base) {
            if (ti < 0)
               continue;
            assert(ti < list.size());
            if (ti < list.size())
               _dec_tex_ref(list[ti], ti);
            ti = -1;
         }
         for (auto& ti : item.shader_params.normals_blends) {
            if (ti < 0)
               continue;
            assert(ti < list.size());
            if (ti < list.size())
               _dec_tex_ref(list[ti], ti);
            ti = -1;
         }
      }
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_landscape_added_or_removed();
   }
   
   namespace {
      void _handle_ni_shader_properties(
         rendered_mesh& mesh,
         const nifDK::block_types::NiAlphaProperty* alpha,
         const nifDK::block_types::BSShaderProperty* shader,
         bool& enable_vertex_alpha,
         bool& enable_vertex_color
      ) {
         if (alpha) {
            mesh.push_params.alpha_test_operation  = (int)alpha->testing.mode;
            mesh.push_params.alpha_test_threshold  = (float)alpha->testing.threshold / 255.0F;
            mesh.push_params.enable_alpha_blending = alpha->blending.enabled ? VK_TRUE : VK_FALSE;
            //
            if (!alpha->testing.enabled)
               mesh.push_params.alpha_test_operation = (int)nifDK::block_types::NiAlphaProperty::test_mode::always;
            if (alpha->blending.enabled)
               mesh.mesh_flags |= rendered_mesh::mesh_flag::requires_oit;
         }
         if (shader) {
            //
            // Skyrim shader flags:
            //
            bool has_shader_flags = false;
            std::array<nifDK::SkyrimShaderPropertyFlags, 2> shader_flags;
            if (auto* casted = dynamic_cast<const nifDK::block_types::BSLightingShaderProperty*>(shader)) {
               shader_flags     = casted->shader_flags;
               has_shader_flags = true;
               //
               if (casted->material.alpha < 1.0) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::requires_oit;
                  //
                  // TODO: pass this alpha value in
                  //
               }
               mesh.shader_params.specular_color    = { casted->specular.color.r, casted->specular.color.g, casted->specular.color.b };
               mesh.shader_params.specular_exponent = casted->material.glossiness;
               mesh.shader_params.specular_strength = casted->specular.strength;
            } else if (auto* casted = dynamic_cast<const nifDK::block_types::BSEffectShaderProperty*>(shader)) {
               shader_flags     = casted->shader_flags;
               has_shader_flags = true;
            }
            if (has_shader_flags) {
               //
               // NOTE: Even with the Vertex Alpha flag enabled, I believe you still need a NiAlphaProperty to 
               // enable alpha blending in order to see vertex alpha values cause any transparency.
               //
               if (shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::vertex_alpha) {
                  enable_vertex_alpha = true;
               }
               if (shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::decal) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::is_decal;
               }
               if (!(shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::cast_shadows)) {
                  mesh.mesh_flags &= ~rendered_mesh::mesh_flag::cast_shadows;
               }
               if (!(shader_flags[0] & nifDK::SkyrimShaderPropertyFlagA::receive_shadows)) {
                  mesh.push_params.receive_shadows = VK_FALSE;
               }
               //
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::double_sided) {
                  mesh.mesh_flags |= rendered_mesh::mesh_flag::double_sided;
               }
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::vertex_colors) {
                  enable_vertex_color = true;
               }
               //
               // For tree meshes, vertex alpha values are co-opted and used for animations:
               //
               if (shader_flags[1] & nifDK::SkyrimShaderPropertyFlagB::use_tree_animation) {
                  enable_vertex_alpha = false;
               }
            }
         }
      }
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
      size_t texture_index = scene::index_of_none;
      size_t normals_index = scene::index_of_none;
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
      if (texture_index != scene::index_of_none) {
         ++this->scene.textures[texture_index].refcount;
         if (prior_diffuse != scene::index_of_none)
            --this->scene.textures[prior_diffuse].refcount;
      }
      if (normals_index != scene::index_of_none) {
         ++this->scene.textures[normals_index].refcount;
         if (prior_normals != scene::index_of_none)
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
      assert(mesh_index != scene::index_of_none);
      auto& mesh       = this->scene.meshes[mesh_index];
      data->vulkan_state.mesh_handle = rendered_mesh_handle(*this, mesh_index);
      //
      mesh.life_state = scene_frame_item_state::active;
      mesh.owning_nif = data->owner;
      mesh.shader_params.transform = transform;
      mesh.texture_indices.diffuse = fallback_texture_index;
      ++this->scene.textures[fallback_texture_index].refcount;
      //
      bool enable_vertex_alpha = false;
      bool enable_vertex_color = false;
      _handle_ni_shader_properties(mesh, data->properties.alpha, data->properties.shader, enable_vertex_alpha, enable_vertex_color);
      {  // Vertices
         mesh.data.vertices.resize(size);
         auto&       list = data->vertices;
         const auto& desc = data->vertex_desc;
         for (size_t i = 0; i < size; ++i) {
            auto& src = list[i];
            auto& dst = mesh.data.vertices[i];
            //
            dst.pos       = src.vertex;
            if (enable_vertex_color) {
               dst.color = { src.color.r, src.color.g, src.color.b, enable_vertex_alpha ? src.color.a : 1.0 };
            } else {
               dst.color = { 1.0, 1.0, 1.0, 1.0 };
            }
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
         mesh.shader_params.bounding_sphere_center = src.center;
         mesh.shader_params.bounding_sphere_radius = src.radius;
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
      auto* geom = dynamic_cast<nifDK::block_types::NiTriBasedGeom*>(object); // the NiGeometry superclass isn't enough for triangle-based rendering
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
      assert(mesh_index != scene::index_of_none);
      auto& mesh       = this->scene.meshes[mesh_index];
      geom->vulkan_state.mesh_handle = rendered_mesh_handle(*this, mesh_index);
      //
      mesh.life_state = scene_frame_item_state::active;
      mesh.owning_nif = object->owner;
      mesh.shader_params.transform = transform;
      mesh.texture_indices.diffuse = fallback_texture_index;
      ++this->scene.textures[fallback_texture_index].refcount;
      //
      bool enable_vertex_alpha = false;
      bool enable_vertex_color = false;
      _handle_ni_shader_properties(mesh, geom->properties.alpha, geom->properties.shader, enable_vertex_alpha, enable_vertex_color);
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
            if (enable_vertex_color && cl.size()) {
               if (cl.size()) {
                  vert.color = { cl[i].r, cl[i].g, cl[i].b, enable_vertex_alpha ? cl[i].a : 1.0 };
               } else {
                  //
                  // If the shader enables vertex colors but the mesh data doesn't actually have any, 
                  // then color it all black.
                  //
                  vert.color = { 0.0, 0.0, 0.0, 1.0 };
               }
            } else {
               vert.color = { 1.0, 1.0, 1.0, 1.0 };
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
         mesh.shader_params.bounding_sphere_center = src.center;
         mesh.shader_params.bounding_sphere_radius = src.radius;
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
   bool surface_renderer::add_nif(nifDK::file& model, const glm::vec3& pos, const glm::vec3& rot, float scale) {
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
         if (texture_index == scene::index_of_none) {
            qDebug("Cannot add new rendered object: failed to add its texture.");
            return false;
         }
         auto& texture_item = this->scene.textures[texture_index];
         texture_item.life_state = scene_frame_item_state::active;
      }
      glm::mat4 transform = glm_transform_from_beth(pos, rot, scale);
      model.root_node->walk_tree(
         transform,
         [](nifDK::block_types::NiNode* node, glm::mat4& transform) {
            transform = transform * node->transform.to_matrix();
         },
         [this, texture_index](nifDK::block_types::NiAVObject* object, const glm::mat4& transform) {
            if (&typeid(*object->parent) == &typeid(nifDK::block_types::NiSwitchNode)) {
               //
               // For NiSwitchNodes, only import the current child. (TODO: Instead, import all children 
               // and then cull the non-active ones somehow.)
               //
               auto* sn = (nifDK::block_types::NiSwitchNode*)object->parent;
               if (sn->current_child() != object)
                  return;
            }
            if (auto* geom = dynamic_cast<nifDK::block_types::NiGeometry*>(object)) {
               this->add_NiGeometry_mesh(geom, transform, texture_index);
            }
            if (auto* geom = dynamic_cast<nifDK::block_types::BSTriShape*>(object)) {
               this->add_BSTriShape_mesh(geom, transform, texture_index);
            }
         }
      );
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
      for (auto& image : this->swap_chain.frames_in_flight) {
         image.on_scene_meshes_added_or_removed();
      }
      return true;
   }
   void surface_renderer::remove_nif(nifDK::file& model) {
      if (!model.root_node) {
         qDebug("[surface_renderer::remove_nif] Model has no root node.");
         return;
      }
      model.root_node->for_non_node_descendants([this](nifDK::block_types::NiAVObject* object) {
         rendered_mesh_handle* handle = nullptr;
         if (auto* intfc = dynamic_cast<nifDK::block_interfaces::_DKVulkanMeshInterface*>(object)) {
            handle = &intfc->vulkan_state.mesh_handle;
         }
         if (handle && !handle->empty()) {
            if (handle->renderer() != this) {
               qDebug("[surface_renderer::remove_nif] WARNING: Geometry object belongs to a different renderer!");
               return;
            }
            handle->destroy();
         }
      });
      for (auto& image : this->swap_chain.frames_in_flight) {
         image.on_scene_meshes_added_or_removed();
      }
   }

   void surface_renderer::set_default_land_textures(const QString& raw_diffuse, const QString& raw_normals) {
      auto diffuse = QDir::cleanPath(raw_diffuse).toLower();
      auto normals = QDir::cleanPath(raw_normals).toLower();
      //
      auto& list = this->scene.textures;
      //
      bool already_diffuse = false;
      bool already_normals = false;
      for (auto& tex : list) {
         if ((tex.flags & loaded_texture::flag::is_default_land_texture) == 0)
            continue;
         if (tex.path == diffuse) {
            already_diffuse = true;
            if (already_normals)
               return;
            continue;
         }
         if (tex.path == normals) {
            already_normals = true;
            if (already_diffuse)
               return;
            continue;
         }
         tex.flags &= ~loaded_texture::flag::is_default_land_texture;
         if (tex.refcount == 0 && !tex.persist_for_life_of_renderer()) {
            //
            // The texture isn't directly used by a rendered_landscape (i.e. it wasn't manually picked 
            // and painted on), and it isn't used by other stuff e.g. rendered_meshes. Mark it for delete 
            // now that we no longer need to keep it.
            //
            ++tex.refcount; // disgusting hack so we can just use dec ref to get consistent mark-for-delete behavior
            this->scene.texture_dec_ref({}, tex);
         }
      }
      //
      if (!already_diffuse) {
         this->scene.global_state.default_land_diffuse_texture = diffuse.isEmpty() ? -1 : this->add_dds_texture(diffuse);
      }
      if (!already_normals) {
         this->scene.global_state.default_land_normals_texture = normals.isEmpty() ? -1 : this->add_dds_texture(normals);
      }
   }

   rendered_light_handle surface_renderer::add_light(dovah::loaded_forms::ObjectReference& refr) {
      auto* base = refr.base_form.get_form_stub();
      if (!base || base->formType != dovah::form_type::light)
         return {};
      auto loaded_base = base->load().ptr_cast<dovah::loaded_forms::Light>();
      if (!loaded_base)
         return {};
      //
      rendered_light::shader_parameters params = {
         .transform = glm_transform_from_beth(refr.position, refr.rotation, 1.0F),
         .color     = {
            (float)loaded_base->color.r / 255.0,
            (float)loaded_base->color.g / 255.0,
            (float)loaded_base->color.b / 255.0,
         },
         .fade    = loaded_base->fade,
         .falloff = loaded_base->falloff_exponent,
         .fov     = loaded_base->fov,
         .radius  = (float)loaded_base->radius,
      };
      if (auto* ex = (dovah::loaded_forms::components::extra::light*)refr.extra_data.lookup_by_type(dovah::loaded_forms::components::extra_data_type::light)) {
         params.fade += ex->fade;
         params.fov  += ex->fov;
      }
      if (auto* ex = (dovah::loaded_forms::components::extra::radius*)refr.extra_data.lookup_by_type(dovah::loaded_forms::components::extra_data_type::radius)) {
         params.radius += ex->value;
      }
      //
      params.type = rendered_light::light_type::omni;
      {
         switch (loaded_base->light_type) {
            using enum dovah::loaded_forms::Light::engine_light_type;
            case omni:
               params.type = rendered_light::light_type::omni;
               break;
            case omni_shadow: // NOTE: we don't yet support shadowing, nor shadowed point lights
               params.type = rendered_light::light_type::omni_shadow;
               break;
            case hemi_shadow:
               params.type = rendered_light::light_type::hemi_shadow;
               break;
            case spot: // Bethesda's spot lights always cast shadows
            case spot_shadow:
               params.type = rendered_light::light_type::spot_shadow;
               break;
            default:
               return {}; // unsupported light type
         }
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_light] Attempting to spawn new light for [REFR:%08X] with base [LIGH:%08X]%s...", refr.stub.formID, base->formID, base->get_editor_id());
      }
      return this->add_light(params);
   }
   rendered_light_handle surface_renderer::add_light(const rendered_light::shader_parameters& in) {
      size_t light_index = this->scene.insert_new_light();
      if (light_index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_light] Cannot add new rendered_light; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_light] Creating new light at index %u.", light_index);
      }
      auto& light = this->scene.lights[light_index];
      light.life_state = scene_frame_item_state::active;
      light.handled_frames.set_all_out_of_date();
      light.shader_params = in;
      light.set_transform(in.transform); // so that transform_inv is valid
      //
      if (light.can_cast_shadows()) {
         this->scene.mark_light_shadows_dirty();
      }
      //
      return rendered_light_handle(*this, light_index);
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

   void surface_renderer::debug_show_frustrums() {
      size_t texture_index;
      {
         texture_index = this->add_texture(QLatin1Literal(":/shaders/white.png"));
         if (texture_index == scene::index_of_none) {
            qDebug("Cannot display debug frustrums: failed to add texture.");
            return;
         }
         auto& texture_item = this->scene.textures[texture_index];
         texture_item.life_state = scene_frame_item_state::active;
      }
      //
      // Show meshes:
      //
      {  // Camera
         auto mesh_index = this->scene.insert_new_mesh();
         if (mesh_index == scene::index_of_none) {
            qDebug("Cannot show debug frustrum (camera). Mesh limit reached.");
            return;
         }
         auto& mesh = this->scene.meshes[mesh_index];
         //
         mesh.life_state = scene_frame_item_state::active;
         mesh.texture_indices.diffuse = texture_index;
         ++this->scene.textures[texture_index].refcount;
         //
         // Vertices:
         //
         {
            constexpr uint16_t NBL = 0;
            constexpr uint16_t NTL = 1;
            constexpr uint16_t NBR = 2;
            constexpr uint16_t NTR = 3;
            constexpr uint16_t FBL = 4;
            constexpr uint16_t FTL = 5;
            constexpr uint16_t FBR = 6;
            constexpr uint16_t FTR = 7;
            //
            auto& list = mesh.data.vertices;
            list.resize(8);
            list[NBL].pos = { -1, -1, 0 }; // near lower left  // Vulkan depth is [0, 1], so that's what we need for our Z
            list[NTL].pos = { -1,  1, 0 }; // near upper left
            list[NBR].pos = {  1, -1, 0 }; // near lower right
            list[NTR].pos = {  1,  1, 0 }; // near upper right
            list[FBL].pos = { -1, -1, 1 }; // far  lower left
            list[FTL].pos = { -1,  1, 1 }; // far  upper left
            list[FBR].pos = {  1, -1, 1 }; // far  lower right
            list[FTR].pos = {  1,  1, 1 }; // far  upper right
            //
            glm::mat4 undo;
            if constexpr (config::use_inverted_depth) {
               //
               // We don't want the frustum to be infinitely long, but inverted depth has no defined 
               // far plane, so we need to compute an alternate projection matrix with standard depth.
               //
               constexpr float near =     0.01F;
               constexpr float far  = 10000.00F;
               //
               auto& src = this->scene.global_state.proj;
               undo = glm::mat4(0);
               undo[0][0] = src[0][0];
               undo[1][1] = src[1][1];
               undo[2][2] = -(far + near) / (far - near);
               undo[2][3] = -1.0F;
               undo[3][2] = -(2.0F * far * near) / (far - near);
               //
               undo *= this->scene.global_state.view;
               undo = glm::inverse(undo);
            } else {
               undo = glm::inverse(this->scene.global_state.proj * this->scene.global_state.view);
            }
            for (size_t i = 0; i < list.size(); ++i) {
               auto& vert = list[i];
               vert.normal    = { 0, 0, 1 };
               vert.tangent   = { 1, 0, 0 };
               vert.bitangent = { 0, 1, 0 };
               vert.uv        = { 0, 0 };
               if (i < 4) {
                  vert.color = { 0.2, 0.2, 0.2, 1.0 }; // near color
               } else {
                  vert.color = { 0.8, 0.8, 0.8, 1.0 }; // far color
               }
               //
               auto posw = undo * glm::vec4(vert.pos, 1.0F);
               vert.pos = posw;
               vert.pos /= posw.w;
            }
            list[NTR].color.r = 1.0;
            list[NBR].color.r = 1.0;
            list[NTL].color.g = 1.0;
            list[NTR].color.g = 1.0;
            //
            // Indices:
            //
            mesh.data.indices = std::array{
               // Left:
               NBL, NTL, FTL,
               FTL, NTL, NBL, // swap winding for double-sided (TODO: use different shader with double-sided polygons)
               //
               FTL, FBL, NBL,
               NBL, FBL, FTL,
               //
               // Right:
               NBR, NTR, FTR,
               FTR, NTR, NBR, // swap winding for doubRe-sided (TODO: use different shader with doubRe-sided poRygons)
               //
               FTR, FBR, NBR,
               NBR, FBR, FTR,
               //
               // Top:
               NTL, NTR, FTR,
               FTR, NTR, NTL,
               //
               FTR, FTL, NTL,
               NTL, FTL, FTR,
               //
               // Bottom:
               NBL, NBR, FBR,
               FBR, NBR, NBL,
               //
               FBR, FBL, NBL,
               NBL, FBL, FBR,
            };
            //
            mesh.recalc_bounding_sphere();
            mesh.shader_params.transform = glm::mat4(1.0F);
         }
         this->_create_mesh_vib(mesh);
         qDebug("Camera debug frustrum added.");
      }
      {  // Sun shadows
         auto mesh_index = this->scene.insert_new_mesh();
         if (mesh_index == scene::index_of_none) {
            qDebug("Cannot show debug frustrum (camera). Mesh limit reached.");
            return;
         }
         auto& mesh = this->scene.meshes[mesh_index];
         //
         mesh.life_state = scene_frame_item_state::active;
         mesh.texture_indices.diffuse = texture_index;
         ++this->scene.textures[texture_index].refcount;
         //
         // Vertices:
         //
         {
            constexpr uint16_t NBL = 0;
            constexpr uint16_t NTL = 1;
            constexpr uint16_t NBR = 2;
            constexpr uint16_t NTR = 3;
            constexpr uint16_t FBL = 4;
            constexpr uint16_t FTL = 5;
            constexpr uint16_t FBR = 6;
            constexpr uint16_t FTR = 7;
            //
            auto& list = mesh.data.vertices;
            list.resize(8);
            list[NBL].pos = { -1, -1, 0 }; // near lower left  // Vulkan depth is [0, 1], so that's what we need for our Z
            list[NTL].pos = { -1,  1, 0 }; // near upper left
            list[NBR].pos = {  1, -1, 0 }; // near lower right
            list[NTR].pos = {  1,  1, 0 }; // near upper right
            list[FBL].pos = { -1, -1, 1 }; // far  lower left
            list[FTL].pos = { -1,  1, 1 }; // far  upper left
            list[FBR].pos = {  1, -1, 1 }; // far  lower right
            list[FTR].pos = {  1,  1, 1 }; // far  upper right
            //
            glm::mat4 undo = glm::inverse(this->scene.global_state.sun_space);
            for (size_t i = 0; i < list.size(); ++i) {
               auto& vert = list[i];
               vert.normal    = { 0, 0, 1 };
               vert.tangent   = { 1, 0, 0 };
               vert.bitangent = { 0, 1, 0 };
               vert.uv        = { 0, 0 };
               if (i < 4) {
                  vert.color = { 0.1, 0.1, 0.1, 1.0 }; // near color
               } else {
                  vert.color = { 0.5, 0.5, 0.5, 1.0 }; // far color
               }
               //
               auto posw = undo * glm::vec4(vert.pos, 1.0F);
               vert.pos = posw;
               vert.pos /= posw.w;
            }
            list[NTR].color.r = 1.0;
            list[NBR].color.r = 1.0;
            list[NTL].color.g = 1.0;
            list[NTR].color.g = 1.0;
            //
            // Indices:
            //
            mesh.data.indices = std::array{
               // Left:
               NBL, NTL, FTL,
               FTL, NTL, NBL, // swap winding for double-sided (TODO: use different shader with double-sided polygons)
               //
               FTL, FBL, NBL,
               NBL, FBL, FTL,
               //
               // Right:
               NBR, NTR, FTR,
               FTR, NTR, NBR, // swap winding for doubRe-sided (TODO: use different shader with doubRe-sided poRygons)
               //
               FTR, FBR, NBR,
               NBR, FBR, FTR,
               //
               // Top:
               NTL, NTR, FTR,
               FTR, NTR, NTL,
               //
               FTR, FTL, NTL,
               NTL, FTL, FTR,
               //
               // Bottom:
               NBL, NBR, FBR,
               FBR, NBR, NBL,
               //
               FBR, FBL, NBL,
               NBL, FBL, FBR,
            };
            //
            mesh.recalc_bounding_sphere();
            mesh.shader_params.transform = glm::mat4(1.0F);
         }
         this->_create_mesh_vib(mesh);
         qDebug("Sun shadow debug frustrum added.");
      }
      for (auto& image : this->swap_chain.frames_in_flight)
         image.on_scene_meshes_added_or_removed();
   }
   void surface_renderer::debug_show_shadow_caster_culling(size_t which) {
      this->debug.show_shadow_caster_culling = which;
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }
   void surface_renderer::debug_set_culling_updates_frozen(bool v) {
      this->debug.freeze_culling_updates = v;
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }

   void surface_renderer::_wait_on_all_frames_in_flight() {
      for (auto& item : this->swap_chain.frames_in_flight)
         item.fences.wait_on_all(this->logical_device);
   }

   void surface_renderer::_execute_pending_scene_deletions() {
      auto  ic = this->swap_chain.images.size();
      auto& pd = this->scene.pending_deletions;
      if (pd.bounds) {
         size_t deleted    =  0;
         size_t last_alive = -1;
         auto&  list       = this->scene.bounds;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date()) {
               item.reset();
               ++deleted;
            } else {
               last_alive = i;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            if (deleted) {
               qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_deletions] Deleted %u scene bounds.", deleted);
            }
         }
         pd.bounds -= deleted;
         list.resize(last_alive + 1);
      }
      if (pd.lights) {
         size_t deleted    =  0;
         size_t last_alive = -1;
         auto&  list       = this->scene.lights;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date()) {
               item.reset();
               ++deleted;
            } else {
               last_alive = i;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            if (deleted) {
               qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_deletions] Deleted %u scene lights.", deleted);
            }
         }
         pd.lights -= deleted;
         list.resize(last_alive + 1);
      }
      if (pd.meshes) {
         size_t deleted    =  0;
         size_t last_alive = -1;
         auto&  list       = this->scene.meshes;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date()) {
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
            if (item.pending_delete() && item.handled_frames.are_all_up_to_date()) {
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