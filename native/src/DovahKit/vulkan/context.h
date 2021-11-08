#pragma once
#include <vector>
#include <QPointer>
#include <QWidget>
#include "device.h"
#include "_vulkan.h"
#include "_util.h"
#include "command_buffer.h"
#include "descriptor_definitions.h"
#include "image.h"
#include "shader_module.h"
#include "swap_chain.h"

// scene:
#include <chrono>
#include "loaded_texture.h"
#include "rendered_mesh.h"

namespace vulkanDK {
   class device;
   class render_pass;
   class swap_chain;

   class context : no_copy {
      public:
         context(device&);
         ~context();

         device& owner;
         descriptor_set_layout descriptor_set_definition;
         //
         VkCommandPool    command_pool    = VK_NULL_HANDLE;
         VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
         //
         VkSurfaceKHR surface = VK_NULL_HANDLE;
         struct {
            QPointer<QWidget> widget;
            bool resized = false;
            bool visible = false;
         } ui;
         VkExtent2D extent;
         //
         swap_chain swap_chain;
         std::vector<render_pass*>   render_passes;
         std::vector<shader_module*> shader_modules;
         VkSampler texture_sampler = VK_NULL_HANDLE;
         concrete_image null_texture;
         //
         struct {
            std::chrono::steady_clock::time_point last_update;
            std::vector<rendered_mesh>  meshes;
            std::vector<loaded_texture> textures;
         } scene;

         inline VkDevice logical_device() const noexcept { return this->owner.logical; }
         inline VkPhysicalDevice physical_device() const noexcept { return this->owner.physical; }

         void setup();

         void handle_resize();

         VkExtent2D current_surface_size() const;
         VkExtent2D desired_surface_size() const;

         template<typename T> inline void do_single_commands(T func) {
            (func)(this->_begin_one_time_commands());
            this->_end_one_time_commands();
         }

         size_t add_texture(const QString& texture_path);
         void add_mesh(const QString& texture_path);
         void remove_mesh(size_t);
         void remove_last_mesh();

      protected:
         void _setup_command_pool();
         void _setup_descriptor_pool(); // requires awareness of the frame-in-flight count
         void _setup_shader_modules();
         void _setup_texture_sampler();
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