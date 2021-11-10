#pragma once
#include "_vulkan.h"
#include "_util.h"
#include "abstract_renderer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "descriptor_definitions.h"
#include "image.h"
#include "scene.h"

namespace vulkanDK {
   class frame_in_flight;
   class render_pass;
   class shader_module;
   class surface;

   class surface_renderer : public abstract_renderer {
      public:
         surface_renderer(surface&, physical_device&);
         ~surface_renderer();

         surface&   target;
         VkExtent2D surface_extent; // last extent we set ourselves up for
         struct {
            queue graphics;
            queue presentation;
         } queues;
         //
         concrete_image null_texture;
         //
         struct {
            VkSwapchainKHR handle = VK_NULL_HANDLE;
            VkFormat       format = VK_FORMAT_UNDEFINED;
            concrete_image depth_buffer; // only one should be needed: we only use it during rendering, not presentation, and we render one frame at a time synched via subpass dependencies
            //
            std::vector<material> materials;
            //
            std::vector<surface_renderer_image_view> images;
            std::vector<VkFramebuffer>   framebuffers;
            std::vector<frame_in_flight> frames_in_flight;
            //
            std::vector<VkFence> images_in_flight; // handles. if images[i] is in flight, then images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle
            size_t current_frame = 0;
         } swap_chain;
         //
         scene scene;

         void setup();
         void teardown();

         void handle_resize();

         void draw_next_frame();

         template<typename T> inline void do_single_commands(T func) {
            (func)(this->_begin_one_time_commands());
            this->_end_one_time_commands();
         }

         VkExtent2D desired_surface_size() const;
         VkFormat find_depth_format() const;
         bool needs_null_texture() const;

         buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags, VkMemoryPropertyFlags);

         // TODO: fully decouple scenes from scene renderers; make it possible to have multiple scene renderers point to the same scene
         size_t add_texture(const QString& texture_path);
         void add_mesh(const QString& texture_path);
         void remove_mesh(size_t);
         void remove_last_mesh();

      protected:
         void _setup_device();
         //
         void _setup_shader_modules();
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
         void _setup_materials(); // requires extent size
         void _setup_depth_buffer(); // requires extent size
         void _setup_swap_chain_images();
         void _setup_framebuffers(); // per swap chain image, and requires each swap chain image's view

         command_buffer _begin_one_time_commands();
         void _end_one_time_commands(command_buffer&);

         void _execute_pending_scene_deletions();
   };
}