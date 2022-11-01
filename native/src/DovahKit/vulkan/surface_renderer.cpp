#include "surface_renderer.h"
#include <array>
#include <chrono>
#include <optional>
#include <typeinfo>
#include <QDir>
#include <QResource> // for loading shaders
#include "helpers/arrays/make.h"
#include "helpers/string/strieq_ascii.h"
#include "helpers/array_concat.h"
#include "helpers/dummy_of.h"
#include "helpers/miscellaneous.h" // cobb::edit_bit
//
#include "./DKVulkanInstance.h"
#include "./compute_shader.h"
#include "./exceptions.h"
#include "./frame_in_flight.h"
#include "./physical_device.h"
#include "./queue_family_info.h"
#include "./raycast.h"
#include "./render_pass.h"
#include "./shader_module.h"
#include "./vertex.h"
#include "./config/frames_in_flight.h"
#include "./config/order_independent_transparency.h"
#include "./config/scene_limits.h"
#include "./config/shadow_maps.h"
#include "./config/throttle_fps.h"
#include "./config/use_inverted_depth.h"
#include "./config/validation_layers.h"
#include "./helpers/convert_access_flags_and_pipeline_stages.h"
#include "./helpers/glm_transform_from_beth.h"
#include "./helpers/specialization_map_entry_for_member.h"

#include "./scene_global_state.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace {
   static constexpr bool debug_object_names_fallback_to_log = false; // logs objects' debug names when the relevant extension isn't supported; log spam on window resize; use only when needed

   // Tool for testing whether the staging buffer size cap works.
   static constexpr bool debug_use_tiny_staging_buffer_for_entity_uploads = false && 
      #if _DEBUG
         true
      #else
         false
      #endif
   ;

   static constexpr bool debug_log_nif_to_meshes_time = true;
   static constexpr bool debug_log_nif_loading_times  = true;

   template<bool Enable>
   using _debug_log_timestamp = cobb::dummy_type_if_false<Enable, std::chrono::time_point<std::chrono::steady_clock>>;
}

namespace {
   const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
      VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME,
   };

   static constexpr auto desired_swap_chain_presentation_mode  = VK_PRESENT_MODE_MAILBOX_KHR;
   static constexpr bool rebuild_swap_chain_asap_if_suboptimal = false;

   static constexpr bool enable_shader_debug_printf = false;
}

#include "overlays/fps.h"

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
      {
         auto& e = pd.support.descriptor_bindings;
         if (!e.runtime_array || !e.variable_count)
            return false;
         if (e.max_bound_descriptor_sets < 8) {
            //
            // TODO: Can we generate this at compile-time from the shaders, somehow? Like, if 
            // we define our shaders as constexpr metadata? I really don't like the idea that 
            // I might forget to update this at some point.
            //
            return false;
         }
         //
         auto& t = e.maximums_by_descriptor_type;
         if (t.input_attachment < 2) // highest needed: 2 for OIT compositing
            return false;
         if (t.uniform_buffer < 2)
            return false;
      }
      if (pd.support.max_vertex_output_components < 0x60 || pd.support.max_fragment_input_components < 0x60)
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
            .shader_stages      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
            .geometryShader    = pd_support.geometry_shaders.available ? VK_TRUE : VK_FALSE,
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
         }
         if constexpr (enable_shader_debug_printf) {
            if (this->device_info->has_extension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)) {
               create_ext.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
            }
         }
         //
         // The extension list may have changed size and therefore address as a result 
         // of further additions, so repoint the create info to it:
         //
         create_info.enabledExtensionCount   = (uint32_t)create_ext.size();
         create_info.ppEnabledExtensionNames = create_ext.data();
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

   void surface_renderer::_setup_debug_grid_index_buffer() {
      constexpr std::array<uint16_t, 6> indices = { 0, 1, 2, 2, 3, 0 };
      constexpr size_t buffer_size = indices.size() * sizeof(decltype(indices)::value_type);

      this->debug_grid_index_buffer = this->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      auto  staging = this->create_staging_buffer(buffer_size);
      void* data    = staging.map_memory();
      memcpy(data, indices.data(), buffer_size);
      staging.unmap_memory(data);
      //
      this->debug_grid_index_buffer.copy_from(staging);
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
      {
         this->uploading.commands = command_buffer(*this);
         {
            auto fence_info = VkFenceCreateInfo{
               .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
               .pNext = nullptr,
               .flags = 0,
            };
            vkCreateFence(this->logical_device, &fence_info, nullptr, &this->uploading.fence);
         }
         {
            auto  staging = this->create_staging_buffer(4); // 4-byte (1px) texture
            void* data    = staging.map_memory();
            ((uint8_t*)data)[0] = 0;
            ((uint8_t*)data)[1] = 0;
            ((uint8_t*)data)[2] = 255;
            ((uint8_t*)data)[3] = 255; // RGBA: 0, 0, 255, 255 i.e. blue
            staging.unmap_memory(data);
            //
            auto& target = this->uploading.pending_texture_placeholder;
            target = owned_image_and_view(*this);
            target.create_image(
               {
                  .extent = {
                     .width  = 1,
                     .height = 1,
                  },
                  .format = VK_FORMAT_R8G8B8A8_SRGB,
                  .usage  = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               },
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
               );
            target.overwrite_from_staging_buffer(staging, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT);
            target.create_basic_view(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
         }
      }
      //
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
      this->_setup_debug_grid_index_buffer(); // requires command pools for the buffer write-via-copy
      this->scene.update_projection(this->surface_extent); // requires extent size
      this->_setup_initial_scene();
      this->_initialize_descriptor_sets();
      //
      this->_on_renderer_ready();
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
         auto staging = this->create_staging_buffer(image_size);
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
      //
      // For now, a no-op; we don't use an initial scene, in part because we don't actually need one 
      // and in part because (possibly due to us not needing one, and me having no real-world case to 
      // think about) I can't figure out the logistics of how we'd clean it up when Worldedit asks us 
      // to start rendering real environments.
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
         auto& list = this->scene.entities_of_type<loaded_texture>();
         auto  size = list.size();
         texture_infos.resize(size);
         for (size_t i = 0; i < size; ++i) {
            texture_infos[i] = {
               .sampler     = VK_NULL_HANDLE,
               .imageView   = list[i].owned_gpu_resources.current.view,
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
            .buffer = frame.shader_params.scene_entity_frame_drawing_data.value_for<rendered_bounds>().handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto landscape_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_entity_frame_drawing_data.value_for<rendered_landscape>().handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE,
         };
         auto rlsp_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_entity_frame_drawing_data.value_for<rendered_light>().handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto rmsp_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.shader_params.scene_entity_frame_drawing_data.value_for<rendered_mesh>().handle,
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
            .buffer = frame.shader_params.scene_entity_frame_culling_data.value_for<rendered_mesh>().handle,
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
         auto culling_mesh_params_main_oit_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.main_oit.params.handle,
            .offset = 0,
            .range  = VK_WHOLE_SIZE, // if you want to always update the whole buffer, you can also pass VK_WHOLE_SIZE
         };
         auto culling_mesh_indices_main_oit_buffer_info = VkDescriptorBufferInfo{
            .buffer = frame.indirect_draw_commands.main_oit.mesh_indices.gpu.handle,
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
               //
               // A descriptor write must write to at least one descriptor, i.e. the 
               // descriptorCount can't be zero. However, if we haven't loaded any 
               // textures yet, then it will be. Work around this with a "dumb" 
               // redundant write (we're storing all our VkWriteDescriptorSet structs 
               // in a fixed-length array, so actually varying their number is tricky 
               // but not worth a refactor).
               //
               if (out[1].descriptorCount == 0)
                  out[1] = out[0];
               //
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
            ([&frame, &frustum_main_buffer_info, &mesh_bounds_buffer_info, &culling_mesh_indices_main_oit_buffer_info, &culling_mesh_params_main_oit_buffer_info]() {
               auto descriptor_set = frame.descriptor_sets.sharing_sets.compute_frustum_culling_main_oit;
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
                     .pBufferInfo      = &culling_mesh_indices_main_oit_buffer_info,
                     .pTexelBufferView = nullptr,
                  },
                  VkWriteDescriptorSet{ // storage buffer object: object index buffer
                     .dstSet           = descriptor_set,
                     .dstArrayElement  = 0,
                     .descriptorCount  = 1,
                     .descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .pImageInfo       = nullptr,
                     .pBufferInfo      = &culling_mesh_params_main_oit_buffer_info,
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
         frame.overlays.fps.initialize_descriptor_sets(*this, frame);
      }
      //
      // Mark textures as synchronized:
      //
      for (auto& entry : this->scene.entities_of_type<loaded_texture>())
         entry.lifetime.sync_state.set_all_up_to_date();
   }
   //
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
         .borderColor      = config::sun_shadow_invert_depth ? VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE : VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
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
            .borderColor      = config::sun_shadow_invert_depth ? VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK : VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
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
         constexpr auto format = config::format_for_oit_accumulator;
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
         constexpr auto format = config::format_for_oit_reveal;
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
      vkDeviceWaitIdle(this->logical_device); // Do not handle errors here; there is no other recourse but attempting a teardown anyway.
      //
      // Ensure all child objects belonging to the instance are destroyed.
      //
      this->scene.teardown(*this);
      this->null_texture.teardown();
      this->debug_grid_index_buffer = {};
      {
         if (auto& fence = this->uploading.fence; fence != VK_NULL_HANDLE) {
            vkDestroyFence(this->logical_device, fence, nullptr);
            fence = VK_NULL_HANDLE;
         }
         this->uploading = {};
      }
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
      scene_entities::all_types::for_each([this]<typename Entity>() {
         if constexpr (!Entity::owned_gpu_resources_are_descriptors) {
            for (auto& item : this->scene.entities_of_type<Entity>())
               item.lifetime.sync_state.set_all_out_of_date();
         }
      });
      //
      // Update surface state:
      //
      this->widget.resized = false;
   }


   void surface_renderer::draw_next_frame() {
      #if _DEBUG
         if (this->debug.debugbreak_queued_on_draw) {
            this->debug.debugbreak_queued_on_draw = false;
            __debugbreak();
         }
      #endif
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
      bool perform_scene_entity_gpu_uploads    = this->_execute_pending_scene_entity_gpu_uploads();
      bool will_perform_nif_multithreaded_load = !this->loading.meshes.empty();
      //
      // If this frame-in-flight is still being used to render and present another swap 
      // chain image, wait for it to finish. We'll also advance the current frame counter 
      // here.
      //
      auto& fif = sc.frames_in_flight[sc.current_frame];
      sc.current_frame = (sc.current_frame + 1) % sc.frames_in_flight.size();
      {
         //
         // But hold on! While we wait, let's get some work done. We may have assets that 
         // we need to load on multiple threads.
         //
         this->_execute_asset_multithreaded_load();
      }
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
      if (perform_scene_entity_gpu_uploads) {
         this->_wait_on_pending_scene_entity_gpu_uploads();
      }
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
         auto diff = time_after - time_prior;
         this->state.last_frame_time = std::chrono::duration<double, std::chrono::seconds::period>(diff).count();
         if constexpr (config::throttle_frame_rate_to_1000) {
            constexpr auto ms = std::chrono::milliseconds{ 1 };
            if (diff < ms) {
               std::this_thread::sleep_for(ms - diff);
            }
         }
         this->state.last_frame_at   = time_after;
         //
         this->state.fps.next_delta(this->state.last_frame_time);
      }
      this->_execute_pending_scene_entity_deletions();
      if (will_perform_nif_multithreaded_load) {
         if (this->hooks.nif_batches.on_background_use_complete) {
            (this->hooks.nif_batches.on_background_use_complete)();
         }
      }
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
   VkDeviceSize surface_renderer::max_upload_buffer_size() const {
      if constexpr (debug_use_tiny_staging_buffer_for_entity_uploads) {
         return 1 * 1024/*B to KB*/;
      }
      return 200 * 1024/*KB to MB*/ * 1024/*B to KB*/;
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

   void surface_renderer::set_animation_paused(size_t i, bool paused) {
      auto& list = this->scene.entities_of_type<rendered_mesh>();
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
      const auto& list = this->scene.entities_of_type<rendered_mesh>();
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
      for (const auto& item : this->scene.entities_of_type<rendered_landscape>()) {
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
   void surface_renderer::do_raycast(raycast& rc) {
      {
         size_t nearest  = -1;
         //
         const auto& list = this->scene.entities_of_type<rendered_mesh>();
         for (size_t i = 0; i < list.size(); ++i) {
            const auto& entity = list[i];
            auto hit = entity.do_raycast(rc);
            if (rc.receive_hit(hit))
               nearest = i;
         }
         if (nearest != -1)
            rc.result.entity = rendered_mesh_handle{ *this, nearest };
      }
      {
         size_t nearest = -1;
         //
         const auto& list = this->scene.entities_of_type<rendered_landscape>();
         for (size_t i = 0; i < list.size(); ++i) {
            const auto& entity = list[i];
            auto hit = entity.do_raycast(rc);
            if (rc.receive_hit(hit))
               nearest = i;
         }
         if (nearest != -1)
            rc.result.entity = rendered_landscape_handle{ *this, nearest };
      }
   }

   void surface_renderer::_queue_mesh_vib_creation(rendered_mesh& mesh) {
      assert(mesh.active() && !mesh.pending_gpu_upload());
      mesh.lifetime.life_state = scene_entities::life_state::active_pending_upload;
      ++this->uploading.pending_upload_counts.value_for<rendered_mesh>();
   }

   void surface_renderer::set_default_land_textures(const QString& raw_diffuse, const QString& raw_normals) {
      auto diffuse = QDir::cleanPath(raw_diffuse).toLower();
      auto normals = QDir::cleanPath(raw_normals).toLower();
      //
      auto& list = this->scene.entities_of_type<loaded_texture>();
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
            this->scene.texture_dec_ref(scene::renderer_passkey{}, tex);
         }
      }
      //
      if (!already_diffuse) {
         auto& ti = this->scene.global_state.default_land_diffuse_texture;
         ti = diffuse.isEmpty() ? -1 : this->add_dds_texture(diffuse);
         if (ti != -1) {
            this->scene.entities_of_type<loaded_texture>()[ti].flags |= loaded_texture::flag::is_default_land_texture;
         }
      }
      if (!already_normals) {
         auto& ti = this->scene.global_state.default_land_normals_texture;
         ti = normals.isEmpty() ? -1 : this->add_dds_texture(normals);
         if (ti != -1) {
            this->scene.entities_of_type<loaded_texture>()[ti].flags |= loaded_texture::flag::is_default_land_texture;
         }
      }
   }
   void surface_renderer::set_landscape_borders_visible(bool v) {
      cobb::edit_bit(this->scene.global_state.flags, scene_global_state::flag::show_landscape_borders, v);
   }

   void surface_renderer::set_debug_grid_visible(bool v) {
      cobb::edit_bit(this->scene.global_state.flags, scene_global_state::flag::show_debug_grid, v);
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
   }
   void surface_renderer::set_camera_position(const glm::vec3& position) {
      this->scene.camera.position = position;
      this->scene.update_camera();
   }

   void surface_renderer::debug_show_frustrums() {
      {  // Camera
         auto mesh_index = this->scene.insert_new_scene_entity<rendered_mesh>();
         if (mesh_index == scene::index_of_none) {
            qDebug("Cannot show debug frustrum (camera). Mesh limit reached.");
            return;
         }
         auto& mesh = this->scene.entities_of_type<rendered_mesh>()[mesh_index];
         mesh.texture_indices.diffuse.set(*this, scene::index_of_none);
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
            auto& list = mesh.mesh_data.vertices;
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
            mesh.mesh_data.indices = std::array{
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
            mesh.frame_drawing_data.transform = glm::mat4(1.0F);
         }
         this->_queue_mesh_vib_creation(mesh);
         qDebug("Camera debug frustrum added.");
      }
      {  // Sun shadows
         auto mesh_index = this->scene.insert_new_scene_entity<rendered_mesh>();
         if (mesh_index == scene::index_of_none) {
            qDebug("Cannot show debug frustrum (camera). Mesh limit reached.");
            return;
         }
         auto& mesh = this->scene.entities_of_type<rendered_mesh>()[mesh_index];
         mesh.texture_indices.diffuse.set(*this, scene::index_of_none);
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
            auto& list = mesh.mesh_data.vertices;
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
            mesh.mesh_data.indices = std::array{
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
            mesh.frame_drawing_data.transform = glm::mat4(1.0F);
         }
         this->_queue_mesh_vib_creation(mesh);
         qDebug("Sun shadow debug frustrum added.");
      }
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_mesh>();
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
   void surface_renderer::debug_set_landscape_wireframes_visible(bool v) {
      this->debug.draw_landscape_wireframe = v;
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }
   void surface_renderer::debug_set_landscape_normals_visible(bool v) {
      this->debug.draw_landscape_normals = v;
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.invalidate_all_command_buffers();
   }
   void surface_renderer::debug_break_on_next_draw() {
      #if _DEBUG
         this->debug.debugbreak_queued_on_draw = true;
      #endif
   }
}