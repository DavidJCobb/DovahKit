#pragma once
#include <array>
#include <chrono>
#include <type_traits>
#include <vector>
#include <QPointer>
#include <QWidget>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"
#include "helpers/debug_helper_typeof.h"
#include "config/scene_limits.h"
#include "abstract_renderer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "descriptor_definitions.h"
#include "image.h"
#include "scene.h"
#include "shader.h"
#include "swap_chain_image.h"
#include "surface_renderer_descriptor_group.h"
//
#include "helpers/constexpr_optional_type.h"
//
#include "widgets/DKVulkanView.h"

class  DKVulkanInstance;
struct DKVulkanCameraUpdate;
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
}
namespace nifDK {
   class file;
   namespace block_types {
      class BSShaderProperty;
      class BSTriShape;
      class NiGeometry;
   }
}

namespace vulkanDK {
   class compute_shader;
   class frame_in_flight;
   class render_pass;
   class shader_module;
   class surface;

   class surface_renderer : public abstract_renderer, no_copy, only_heap_allocate {
      friend class DKVulkanView;
      public:
         surface_renderer(DKVulkanInstance&, DKVulkanView*);
         ~surface_renderer();

         static constexpr shader::id_type oit_composite_shader_id    = "OITCompo";
         static constexpr shader::id_type main_shader_id             = "MainMatl";
         static constexpr shader::id_type main_shader_oit_color_id   = "MainOITc";
         static constexpr shader::id_type sun_shadow_shader_id       = "SunShadw";
         static constexpr shader::id_type frustum_cull_shader_id     = "CullFstm";
         static constexpr shader::id_type light_shadow_map_shader_base_id = "LiteSdw0";

         using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::ratio<1>>>;
         
         static constexpr auto color_target_layout_for_render = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
         static constexpr auto color_target_access_for_render = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

         static constexpr size_t shadow_caster_count = config::max_active_shadow_casters;

      protected:
         // renderer events:
         void _on_renderer_ready();
         void _on_renderer_teardown_imminent();
         void _on_renderer_teardown_complete();

         // widget events:
         void _on_repaint();
         void _on_visibility_change(QSize, bool visible); // or resize

         struct shadow_cast_resources {
            size_t light_index = std::string::npos;
            owned_image_and_view cubemap;
         };

      public:
         // APIs
         static bool device_is_supported(const physical_device&);
         void set_physical_device(const physical_device&);
         void set_widget(DKVulkanView*);
         void update_widget_id();

      public:
         DKVulkanInstance& owner;
         VkSurfaceKHR      handle = VK_NULL_HANDLE;
         struct {
            QPointer<DKVulkanView> pointer;
            bool resized = false;
            bool visible = false;
            WId  last_id = {};
         } widget;
         //
         VkExtent2D surface_extent; // last extent we set ourselves up for
         struct {
            queue compute;
            queue graphics;
            queue presentation;
         } queues;
         //
         cobb::constexpr_optional<VmaAllocator, use_vma_library> allocator;
         //
         owned_image_and_view null_texture;
         VkSampler raw_pixel_texture_sampler;
         //
         descriptor_set_layout_group descriptor_set_layouts;
         struct {
            //
            // These resources can be per-renderer rather  than per-swap chain image because they're 
            // only used on the GPU during rendering, not presentation, and we render one frame at a 
            // time synched by subpass dependencies. The only action that the CPU takes with respect 
            // to these resources is to queue GPU-side actions by way of command buffers.
            // 
            // Things that might be accessed or modified on the CPU need to be per-swap chain image, 
            // because the GPU may still be using those  resources for a frame that's been submitted 
            // for presentation. That will mainly apply to descriptor sets and their contents.
            //
            VkFramebuffer        main_framebuffer = VK_NULL_HANDLE;
            owned_image_and_view depth;
            owned_image_and_view color;
            struct {
               owned_image_and_view map;
               VkFramebuffer framebuffer = VK_NULL_HANDLE;
               VkSampler     sampler     = VK_NULL_HANDLE;
            } sun_shadow;
            struct {
               VkFramebuffer framebuffer = VK_NULL_HANDLE;
               VkSampler     sampler     = VK_NULL_HANDLE;
               std::array<shadow_cast_resources, shadow_caster_count> resources;
            } light_shadows;
            struct {
               VkFramebuffer framebuffer = VK_NULL_HANDLE;
               owned_image_and_view accumulator;
               owned_image_and_view reveal;
            } oit;
         } canvas;
         struct {
            VkSwapchainKHR handle = VK_NULL_HANDLE;
            VkFormat       format = VK_FORMAT_UNDEFINED;
            //
            std::vector<swap_chain_image> images;
            std::vector<frame_in_flight>  frames_in_flight;
            //
            size_t current_frame = 0;
         } swap_chain;
         //
         std::vector<shader*> shaders;
         std::vector<compute_shader*> compute_shaders;
         union {
            std::array<render_pass*, 5> _list = { nullptr, nullptr, nullptr, nullptr, nullptr };
            struct {
               render_pass* main_shadow; // shadows for the directional sun
               render_pass* main_shadow_placed; // shadows for placed lights
               render_pass* main;
               render_pass* main_oit;
               render_pass* ui;
            };
         } render_passes_by_name;
         //
         scene scene;
         //
         struct {
            timestamp_t last_frame_at;
            double last_frame_time = 0.0;
         } state;
         //
         struct {
            PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
         } api_functions;
         struct {
            size_t show_shadow_caster_depths = std::string::npos;
         } debug;

         void setup(); // sets up the device and everything below, but not the surface
         void teardown(); // tears down the device and everything below, but not the surface

         void handle_resize();

         void draw_next_frame();

         template<typename T> inline void do_single_commands(T func) {
            auto cb = this->_begin_one_time_commands();
            (func)(cb);
            this->_end_one_time_commands(cb);
         }

         bool can_do_alpha() const;
         VkExtent2D desired_surface_size() const;
         VkFormat find_depth_format() const;
         bool needs_null_texture() const;

         buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);
         void set_debug_object_name(uint64_t handle, VkObjectType type, const std::string& name);
         template<typename T> void set_debug_object_name(T handle, const std::string& name) {
            this->set_debug_object_name((uint64_t)handle, debug_helper_typeof<T>, name);
         }

         shader* get_shader(cobb::eight_cc id) const;
         shader* get_or_create_shader(cobb::eight_cc id);

         compute_shader* create_compute_shader(cobb::eight_cc id);
         compute_shader* get_compute_shader(cobb::eight_cc id) const;

         // TODO: fully decouple scenes from scene renderers; make it possible to have multiple scene renderers point to the same scene
         size_t add_texture(const QString& texture_path);
         size_t add_dds_texture(QString texture_path); // path should be relative to, and not include, "data/"; fails if it doesn't start with "textures/"
         void add_mesh(const QString& texture_path);
         void remove_mesh(size_t);
         void remove_last_mesh();
         void remove_light(size_t);
         void remove_last_light();
         void set_animation_paused(size_t mesh, bool paused);
         //
         size_t object_index_at(int viewport_x, int viewport_y); // returns -1 if none
      
      protected:
         void _create_mesh_vib(rendered_mesh&);
         void _handle_ni_textures(rendered_mesh&, nifDK::block_types::BSShaderProperty*);
         void add_BSTriShape_mesh(nifDK::block_types::BSTriShape* object, glm::mat4 transform, size_t fallback_texture_index);
         void add_NiGeometry_mesh(nifDK::block_types::NiGeometry* object, glm::mat4 transform, size_t fallback_texture_index);
      public:
         bool add_nif(nifDK::file& model, const glm::vec3& pos = glm::vec3(0, 0, 0), const glm::vec3& rot = glm::vec3(0, 0, 0), float scale = 1.0F);
         bool add_light(dovah::loaded_forms::ObjectReference&); // just for testing purposes
         bool add_light(const rendered_light::shader_parameters&);

         void move_camera(const glm::vec3& move, const glm::vec3& turn_euler);
         void set_camera_position(const glm::vec3& pos);

         inline double last_frame_time() const { return this->state.last_frame_time; }

         void debug_show_frustrums(); // adds relevant frustrums to the scene as rendered_meshes.
         void debug_show_shadow_caster_depth(size_t which = std::string::npos);

      protected:
         void _init_surface(); // on init, and when the HWND changes
         void _init_device();
         void _reset_surface();

         void _setup_raw_pixel_texture_sampler();
         void _teardown_raw_pixel_texture_sampler();
         
         void _define_render_passes(); // creates the render_pass wrappers; however, the data needed to instantiate wrapped VkRenderPasses won't be available yet (see _setup_render_passes)
         void _setup_shaders();        // requires that the render pass wrappers exist; wrapped VkRenderPasses don't need to exist yet
            void _setup_oit_composite_shader(); // OITCompo
            void _setup_basic_color_shader();   // MainMatl
            void _setup_basic_wboit_shader();   // MainOITc
            void _setup_sun_shadow_shader();    // SunShadw
            void _setup_light_shadow_shaders(); // LiteMap0 - LiteMap8
            void _setup_light_shadow_debug_shaders(); // DBGLite0 - DBGLite3
            void _setup_frustum_cull_shader();
         //
         void _create_null_texture(); // requires command pool
         void _setup_initial_scene(); // requires command pool for textures
         void _initialize_descriptor_sets(); // requires frame_in_flight::setup_descriptor_sets
         //
         void _setup_render_passes(); // requires awareness of the swap chain format; must rebuild if that format has changed
         //
         // Swap chain setup:
         //
         void _setup_swap_chain_instance();
         void _setup_depth_buffer(); // requires surface extent
         void _setup_color_buffer(); // requires surface extent
         void _setup_sun_shadow_buffer();
         void _setup_light_shadow_resources(); // sets up depth images, samplers, and framebuffers. requires render passes
         void _setup_oit_images(); // requires surface extent
         void _setup_swap_chain_images();
         void _setup_frames_in_flight();
         void _setup_framebuffers(); // requires surface extent
         //
         void _setup_descriptor_pool();

         command_buffer _begin_one_time_commands();
         void _end_one_time_commands(command_buffer&);

         void _execute_pending_scene_deletions();
   };
}