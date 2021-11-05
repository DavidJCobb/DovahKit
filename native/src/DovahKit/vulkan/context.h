#pragma once
#include <vector>
#include <QPointer>
#include <QWidget>
#include "device.h"

namespace DovahKit::vulkan {
   class context;
   class material;

   class surface {
      public:
         VkSurfaceKHR      handle;
         QPointer<QWidget> widget;
         
         QSize last_size;
         bool  resized = false;
         bool  visible = false;

         uint32_t width() const;
         uint32_t height() const;
         inline VkExtent2D extent() const { return VkExtent2D{ this->width(), this->height() }; }
   };

   
   struct swap_chain_support_info {
      VkSurfaceCapabilitiesKHR        capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR>   presentation_modes;
      //
      swap_chain_support_info() {}
      swap_chain_support_info(VkPhysicalDevice, const surface&);
   };

   class swap_chain {
      public:
         context*       owner  = nullptr;
         VkSwapchainKHR handle = VK_NULL_HANDLE;
         VkFormat       format;
         VkExtent2D     extent;
         VkRenderPass   render_pass = VK_NULL_HANDLE;
         struct {
            VkPipelineLayout layout = VK_NULL_HANDLE;
            VkPipeline       handle = VK_NULL_HANDLE;
         } pipeline;
         std::vector<VkImage>        images;
         std::vector<VkImageView>    views;
         std::vector<VkFramebuffer>  framebuffers;
         std::vector<VkBuffer>       uniform_buffers;
         std::vector<VkDeviceMemory> uniform_buffer_memory;
         //
         std::vector<VkFence> images_in_flight; // handles. if swap_chain.images[i] is in flight, then swap_chain.images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle

         std::vector<material*> known_materials;

      public:
         swap_chain();
         ~swap_chain();

         void set_owner(context*);

         void add_material(material*);
         void remove_material(material*);

         void setup();
         void teardown(); // TODO: needs to somehow notify dependent systems, e.g. rendered_objects which have one descriptor of each type per swap chain image, and one descriptor set layout per swap chain image

         inline size_t image_count() const noexcept { return this->images.size(); }

      protected:
         void setup_basics();
         void setup_views();
         void setup_render_pass();
         void setup_pipeline(); // requires knowledge of the dimensions to render to; requires the descriptor set layout
         void setup_depth_buffer();
         void setup_framebuffers();
         void setup_uniform_buffers(); // requires one buffer per swap chain image
   };

   class context {
      public:
         device&  devices;
         surface* surface = nullptr;
         struct {
            VkQueue graphics;
            VkQueue presentation;
         } queues;
         swap_chain*   swap_chain   = nullptr;
         VkCommandPool command_pool = VK_NULL_HANDLE;

         context(device&);
         ~context();

         void set_surface(surface*);

      protected:
   };
}