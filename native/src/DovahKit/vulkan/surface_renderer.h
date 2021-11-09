#pragma once
#include "_vulkan.h"
#include "_util.h"
#include "abstract_renderer.h"
#include "buffer.h"
#include "command_buffer.h"
#include "descriptor_definitions.h"
#include "image.h"
#include "swap_chain.h"
#include "scene.h"

namespace vulkanDK {
   class logical_device;
   class render_pass;
   class shader_module;
   class surface;

   class surface_renderer : public abstract_renderer {
      public:
         struct queue {
            VkQueue  handle = VK_NULL_HANDLE;
            uint32_t index  = 0; // family index

            void setup(VkDevice, uint32_t);
         };
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
         swap_chain swap_chain;
         concrete_image null_texture;
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

         command_buffer _begin_one_time_commands();
         void _end_one_time_commands(command_buffer&);
   };
}