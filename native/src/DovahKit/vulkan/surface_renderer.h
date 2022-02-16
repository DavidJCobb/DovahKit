#pragma once
#include <QPointer>
#include <QWidget>
#include <glm/glm.hpp>
#include "_vulkan.h"
#include "_memory.h"
#include "_util.h"
#include "abstract_renderer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "descriptor_definitions.h"
#include "image.h"
#include "scene.h"
#include "shader.h"
#include "swap_chain_image.h"
//
#include "helpers/constexpr_optional_type.h"
//
#include "widgets/DKVulkanView.h"

class  DKVulkanInstance;
struct DKVulkanCameraUpdate;
namespace nifDK {
   class file;
   namespace block_types {
      class BSShaderProperty;
      class BSTriShape;
      class NiGeometry;
   }
}

namespace vulkanDK {
   class frame_in_flight;
   class render_pass;
   class shader_module;
   class surface;

   class surface_renderer : public abstract_renderer, no_copy, only_heap_allocate {
      friend class DKVulkanView;
      public:
         surface_renderer(DKVulkanInstance&, DKVulkanView*);
         ~surface_renderer();

         static constexpr shader::id_type main_shader_id = "MainMatl";

      protected:
         // renderer events:
         void _on_renderer_ready();
         void _on_renderer_teardown_imminent();
         void _on_renderer_teardown_complete();

         // widget events:
         void _on_repaint();
         void _on_visibility_change(QSize, bool visible); // or resize

      public:
         // APIs
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
            queue graphics;
            queue presentation;
         } queues;
         //
         cobb::constexpr_optional<VmaAllocator, use_vma_library> allocator;
         //
         concrete_image null_texture;
         VkSampler raw_pixel_texture_sampler;
         //
         struct {
            VkSwapchainKHR handle = VK_NULL_HANDLE;
            VkFormat       format = VK_FORMAT_UNDEFINED;
            concrete_image depth_buffer; // only one should be needed: we only use it during rendering, not presentation, and we render one frame at a time synched via subpass dependencies
            //
            std::vector<swap_chain_image> images;
            std::vector<frame_in_flight>  frames_in_flight;
            //
            size_t current_frame = 0;
         } swap_chain;
         //
         std::vector<shader*> shaders;
         union {
            std::array<render_pass*, 2> _list = { nullptr, nullptr };
            struct {
               render_pass* main;
               render_pass* ui;
            };
         } render_passes_by_name;
         //
         scene scene;
         //
         struct {
            double last_frame_time = 0.0;
         } state;

         void setup(); // sets up the device and everything below, but not the surface
         void teardown(); // tears down the device and everything below, but not the surface

         void handle_resize();

         void draw_next_frame();

         template<typename T> inline void do_single_commands(T func) {
            auto cb = this->_begin_one_time_commands();
            (func)(cb);
            this->_end_one_time_commands(cb);
         }

         VkExtent2D desired_surface_size() const;
         VkFormat find_depth_format() const;
         bool needs_null_texture() const;

         buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);

         shader* get_shader(cobb::eight_cc id) const;
         shader* get_or_create_shader(cobb::eight_cc id);

         // TODO: fully decouple scenes from scene renderers; make it possible to have multiple scene renderers point to the same scene
         size_t add_texture(const QString& texture_path);
         size_t add_dds_texture(QString texture_path); // path should be relative to, and not include, "data/"; fails if it doesn't start with "textures/"
         void add_mesh(const QString& texture_path);
         void remove_mesh(size_t);
         void remove_last_mesh();
         void set_animation_paused(size_t mesh, bool paused);
         //
         size_t object_index_at(int viewport_x, int viewport_y); // returns -1 if none
      
      protected:
         void _create_mesh_vib(rendered_mesh&);
         void _handle_ni_textures(rendered_mesh&, nifDK::block_types::BSShaderProperty*);
         void add_BSTriShape_mesh(nifDK::block_types::BSTriShape* object, glm::mat4 transform, size_t fallback_texture_index);
         void add_NiGeometry_mesh(nifDK::block_types::NiGeometry* object, glm::mat4 transform, size_t fallback_texture_index);
      public:
         bool add_nif(nifDK::file& model);

         void move_camera(const glm::vec3& move, const glm::vec3& turn_euler);
         void set_camera_position(const glm::vec3& pos);

         inline double last_frame_time() const { return this->state.last_frame_time; }

      protected:
         void _init_surface(); // on init, and when the HWND changes
         void _init_device();
         void _reset_surface();

         void _setup_raw_pixel_texture_sampler();
         void _teardown_raw_pixel_texture_sampler();
         
         void _define_render_passes(); // creates the render_pass wrappers; however, the data needed to instantiate wrapped VkRenderPasses won't be available yet (see _setup_render_passes)
         void _setup_shaders();        // requires that the render pass wrappers exist; wrapped VkRenderPasses don't need to exist yet
         //
         void _create_null_texture(); // requires command pool
         void _setup_initial_scene(); // requires command pool for textures
         void _initialize_descriptor_sets();
         //
         void _setup_render_passes(); // requires awareness of the swap chain format; must rebuild if that format has changed
         //
         // Swap chain setup:
         //
         void _setup_swap_chain_instance();
         void _setup_depth_buffer(); // requires extent size
         void _setup_swap_chain_images();
         void _setup_swap_chain_image_frame_data(); // requires descriptor pool
         void _setup_framebuffers(); // per swap chain image, and requires each swap chain image's view

         command_buffer _begin_one_time_commands();
         void _end_one_time_commands(command_buffer&);

         void _execute_pending_scene_deletions();
   };
}