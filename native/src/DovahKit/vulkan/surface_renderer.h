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
#include "config/frames_in_flight.h"
#include "config/scene_limits.h"
#include "abstract_renderer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "compute_shader.h"
#include "descriptor_definitions.h"
#include "fps_tracker.h"
#include "graphics_shader.h"
#include "image.h"
#include "scene.h"
#include "scene_entity_handle.h"
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
      class Landscape;
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
   class raycast;
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

         static constexpr graphics_shader::id_type oit_composite_shader_id    = "OITCompo";
         static constexpr graphics_shader::id_type scene_background_shader_id = "ScnBgClr";
         static constexpr graphics_shader::id_type mesh_color_base_shader_id  = "MeshColr";
         static constexpr graphics_shader::id_type mesh_color_oit_shader_id   = "MeshOITc";
         static constexpr graphics_shader::id_type shader_id_mesh_shadows_sun    = "MeshSdwS";
         static constexpr graphics_shader::id_type shader_id_mesh_shadows_caster = "MeshSdw0";
         static constexpr compute_shader::id_type  frustum_cull_shader_id     = "MeshClFs";
         static constexpr compute_shader::id_type  shadow_caster_cull_shader_base_id = "MeshClS0";
         static constexpr graphics_shader::id_type bounding_box_shader_id     = "BoundBox";
         static constexpr graphics_shader::id_type bounding_origin_shader_id  = "BoundPvt";
         static constexpr graphics_shader::id_type landscape_shader_id           = "LandColr";
         static constexpr graphics_shader::id_type shader_id_landscape_shadows_sun    = "LandSdwS";
         static constexpr graphics_shader::id_type shader_id_landscape_shadows_caster = "LandSdw0";
         static constexpr graphics_shader::id_type landscape_border_shader_id    = "LandBrdr";
         static constexpr graphics_shader::id_type landscape_wireframe_shader_id = "LandWire";
         static constexpr graphics_shader::id_type landscape_normals_shader_id   = "LandNrml"; // graphics_shader instance will not exist if geometry shaders aren't available on this hardware
         static constexpr graphics_shader::id_type debug_grid_color_shader_id = "GridColr";

         using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::chrono::seconds::period>>;
         
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
         union _ {
            _() { for (auto& q : list) q = queue(); } // C++ union limits are really dumb sometimes
            //
            std::array<queue, 4> list;
            struct {
               queue compute;
               queue graphics;
               queue presentation;
               queue transfer;
            };
         } queues;
         //
         cobb::constexpr_optional<VmaAllocator, use_vma_library> allocator;
         //
         buffer debug_grid_index_buffer;
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
            command_buffer commands;
            VkFence        fence = VK_NULL_HANDLE;
            buffer         staging;
            //
            cobb::class_map_from_class_array<size_t, scene_entities::all_types_with_owned_gpu_resources> pending_upload_counts;
            
            // At some point, we want to implement spreading out GPU data uploads over multiple frames. 
            // If we have 500 MB of data to upload, we shouldn't rig up a 500 MB staging buffer and use 
            // 1 GB VRAM concurrently just to get all that data onto the GPU in a single frame, right?
            // 
            // Once we do, we'll need to manage asynchronicity with respect to textures. The easiest 
            // way is: when a texture is marked as pending, immediately update descriptors to upload a 
            // placeholder texture, so that meshes which use the texture don't fail to render. Then, 
            // when the texture loads, update descriptors again.
            // 
            // This here is that placeholder texture.
            owned_image_and_view pending_texture_placeholder;
         } uploading;
         struct {
            VkSwapchainKHR handle = VK_NULL_HANDLE;
            VkFormat       format = VK_FORMAT_UNDEFINED;
            //
            std::vector<swap_chain_image> images;
            std::array<frame_in_flight, config::frames_in_flight_count> frames_in_flight;
            //
            size_t current_frame = 0;
         } swap_chain;
         //
         std::vector<graphics_shader*> graphics_shaders;
         std::vector<compute_shader*>  compute_shaders;
         union {
            std::array<render_pass*, 6> _list = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
            struct {
               render_pass* main_shadow; // shadows for the directional sun
               render_pass* main_shadow_placed; // shadows for placed lights
               render_pass* main;
               render_pass* main_oit;
               render_pass* bounds;
               render_pass* ui;
            };
         } render_passes_by_name;
         //
         scene scene;
         //
         struct {
            fps_tracker fps;
            timestamp_t last_frame_at;
            double      last_frame_time = 0.0;
         } state;
         //
         struct {
            PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
         } api_functions;
         struct {
            bool debugbreak_queued_on_draw = false;
            //
            bool   freeze_culling_updates     = false;
            size_t show_shadow_caster_culling = std::string::npos;
            //
            bool draw_landscape_wireframe = false;
            bool draw_landscape_normals   = false; // TODO
         } debug;
         VkFence one_time_commands_fence = VK_NULL_HANDLE;

         void setup(); // sets up the device and everything below, but not the surface
         void teardown(); // tears down the device and everything below, but not the surface

         void handle_resize();

         void draw_next_frame();

         template<typename T> inline void do_single_commands(T func) {
            this->do_single_commands(func, this->queues.graphics);
         }
         template<typename T> inline void do_single_commands(T func, queue& q) {
            auto cb = this->_begin_one_time_commands(q);
            (func)(cb);
            this->_end_one_time_commands(cb, q);
         }

         bool can_do_alpha() const;
         VkExtent2D desired_surface_size() const;
         VkFormat find_depth_format() const;
         bool needs_null_texture() const;

         [[nodiscard]] buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);
         [[nodiscard]] inline buffer create_staging_buffer(VkDeviceSize size) {
            return this->create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
         }
         void set_debug_object_name(uint64_t handle, VkObjectType type, const std::string& name);
         template<typename T> void set_debug_object_name(T handle, const std::string& name) {
            this->set_debug_object_name((uint64_t)handle, debug_helper_typeof<T>, name);
         }

         [[nodiscard]] graphics_shader* get_graphics_shader(cobb::eight_cc id) const;
         [[nodiscard]] graphics_shader* create_graphics_shader(cobb::eight_cc id);

         [[nodiscard]] compute_shader* create_compute_shader(cobb::eight_cc id);
         [[nodiscard]] compute_shader* get_compute_shader(cobb::eight_cc id) const;

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
         void surface_position_to_world_ray(int viewport_x, int viewport_y, glm::vec3& eye_position, glm::vec3& eye_direction) const;
         rendered_mesh_handle rendered_mesh_at(int viewport_x, int viewport_y); // returns -1 if none
         void do_raycast(raycast&);

         rendered_bounds_handle add_bounds(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform);
         void remove_bounds(size_t);

         rendered_landscape_handle add_landscape(const glm::vec3& position);
         rendered_landscape_handle add_landscape(const glm::vec3& position, const dovah::loaded_forms::Landscape&);
         void remove_landscape(size_t);
      
      protected:
         void _queue_mesh_vib_creation(rendered_mesh&);
         void _handle_ni_textures(rendered_mesh&, nifDK::block_types::BSShaderProperty*);
         rendered_mesh* add_BSTriShape_mesh(nifDK::block_types::BSTriShape* object, glm::mat4 transform, size_t fallback_texture_index);
         rendered_mesh* add_NiGeometry_mesh(nifDK::block_types::NiGeometry* object, glm::mat4 transform, size_t fallback_texture_index);
      public:
         bool add_nif(nifDK::file& model, const glm::vec3& pos = glm::vec3(0, 0, 0), const glm::vec3& rot = glm::vec3(0, 0, 0), float scale = 1.0F);
         rendered_light_handle add_light(dovah::loaded_forms::ObjectReference&); // just for testing purposes
         rendered_light_handle add_light(const rendered_light::frame_drawing_data_type&);
         void remove_nif(nifDK::file& model);

         void set_default_land_textures(const QString& diffuse, const QString& normals); // path should be relative to, and not include, "data/"; fails if it doesn't start with "textures/"
         void set_landscape_borders_visible(bool);

         void set_debug_grid_visible(bool);

         void move_camera(const glm::vec3& move, const glm::vec3& turn_euler);
         void set_camera_position(const glm::vec3& pos);

         inline double last_frame_time() const { return this->state.last_frame_time; }

         void debug_show_frustrums(); // adds relevant frustrums to the scene as rendered_meshes.
         void debug_show_shadow_caster_culling(size_t which = std::string::npos);
         void debug_set_culling_updates_frozen(bool);
         void debug_set_landscape_wireframes_visible(bool);
         void debug_set_landscape_normals_visible(bool);
         void debug_break_on_next_draw();

      protected:
         void _init_surface(); // on init, and when the HWND changes
         void _init_device();
         void _reset_surface();

         void _setup_raw_pixel_texture_sampler();
         void _teardown_raw_pixel_texture_sampler();

         void _setup_debug_grid_index_buffer();
         
         void _define_render_passes(); // creates the render_pass wrappers; however, the data needed to instantiate wrapped VkRenderPasses won't be available yet (see _setup_render_passes)
         void _setup_shaders();        // requires that the render pass wrappers exist; wrapped VkRenderPasses don't need to exist yet
            void _setup_scene_background_shader();
            void _setup_oit_composite_shader();
            void _setup_rendered_mesh_shaders();
               void _setup_rendered_mesh_color_shader();
               void _setup_rendered_mesh_wboit_shader();
               void _setup_rendered_mesh_shadows_caster_shaders();
               void _setup_rendered_mesh_shadows_sun_shader();
            void _setup_rendered_landscape_shaders();
               void _setup_rendered_landscape_color_shader();
               void _setup_rendered_landscape_shadows_caster_shaders();
               void _setup_rendered_landscape_shadows_sun_shader();
               void _setup_rendered_landscape_border_shader();
               void _setup_rendered_landscape_wireframe_shader();
               void _setup_rendered_landscape_normals_shader();
            void _setup_frustum_cull_shader();
            void _setup_shadow_caster_cull_shaders();
            void _setup_scene_bounds_shaders();
            void _setup_debug_grid_shader();
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

         command_buffer _begin_one_time_commands(queue&);
         void _end_one_time_commands(command_buffer&, queue&);

         void _wait_on_all_frames_in_flight();

         [[nodiscard]] bool _execute_pending_scene_entity_gpu_uploads(); // returns true if any commands are recorded
         void _wait_on_pending_scene_entity_gpu_uploads();
         void _execute_pending_scene_entity_deletions();
   };
}