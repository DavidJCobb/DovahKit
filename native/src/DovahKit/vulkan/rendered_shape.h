#pragma once
#include <cstdint>
#include <forward_list>
#include <vector>
#include <glm/glm.hpp>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//
// trying to work out a mental model of what's needed to render a single shape.
//

namespace DovahKit::vulkan {
   class device_buffer_heap;
   class context {
      public:
         struct {
            VkPhysicalDevice physical;
            VkDevice         logical;
            //
            struct {
               float anisotropic_filtering_max = 0.0;
            } support;
         } device;
         VkCommandPool command_pool;
         struct {
            VkQueue graphics;
            VkQueue presentation;
         } queues;
         device_buffer_heap* buffer_heap = nullptr;

         // TODO: requires complete teardown
         void setDevice(VkPhysicalDevice);
   };


   struct texture;

   class independent_buffer {
      protected:
         VkDevice device;
      public:
         VkBuffer       handle = 0;
         VkDeviceMemory memory = 0;

         independent_buffer(VkDevice);
         ~independent_buffer();

         independent_buffer(const independent_buffer&) = delete;
         independent_buffer& operator=(const independent_buffer&) = delete;
         independent_buffer(independent_buffer&&);
         independent_buffer& operator=(independent_buffer&&);
   };

   class buffer;
   class device_buffer_heap;
   //
   class device_buffer_heap {
      friend class buffer;
      public:
         static constexpr uint32_t page_size_in_mb = 256;
         static constexpr bool     do_not_share_mappable_pages = true;
      protected:
         VkDevice device;
         uint32_t buffer_image_granularity;

         struct range {
            VkDeviceSize start  = 0;
            VkDeviceSize size   = 0;
            bool         linear = false; // hardware may require "linear" and "non-linear" resources to have gaps between them
            //
            inline VkDeviceSize end() const noexcept { return this->start + this->size; }
         };

         struct page {
            VkDeviceMemory memory;
            VkDeviceSize   size;
            std::forward_list<range> occupied; // sorted
            //
            // The vkMapMemory function cannot be used to concurrently map multiple regions within the 
            // same VkDeviceMemory. That is: if you use vkMapMemory to map ANY PART of a VkDeviceMemory 
            // region, then you cannot map ANY OTHER PART of that VkDeviceMemory until you unmap the 
            // already-mapped region.
            // 
            // Hypothetically, we could solve this by putting a recursive_mutex on each page, and then 
            // requiring map and unmap operations to go through member functions on (buffer) which would 
            // access the page of origin and manage that mutex. It's simpler, however, to just demand 
            // that any mappable memory (properties other than VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) have 
            // a page entirely to itself. Relevant code checks (do_not_share_mappable_pages).
            //
            bool mappable = false;
            //
            bool request(VkMemoryRequirements, bool linear, uint32_t buffer_image_granularity, VkDeviceSize& offset);
            void release(VkDeviceSize offset);
         };

         struct pool {
            VkMemoryType type;
            uint32_t     type_index;
            std::forward_list<page> pages;
            //
            page& create_page(VkDevice);
         };

         std::vector<pool> pools;

      public:
         device_buffer_heap(VkDevice, VkPhysicalDevice, uint32_t buffer_image_granularity);
         ~device_buffer_heap() {}

         buffer create_buffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags);

         VkAllocationCallbacks callbacks();
   };
   //
   class buffer {
      friend class device_buffer_heap;
      protected:
         device_buffer_heap* owner;
         device_buffer_heap::page* page;

         buffer(device_buffer_heap&, device_buffer_heap::page&, VkDeviceSize offset, VkDeviceSize size);

         VkDeviceSize _offset;
         VkDeviceSize _size;

      public:
         VkBuffer handle = 0;

         ~buffer();

         buffer(const buffer&) = delete;
         buffer& operator=(const buffer&) = delete;
         buffer(buffer&&) noexcept;
         buffer& operator=(buffer&&) noexcept;

         inline bool exists() const noexcept { return this->handle != 0; }
         inline VkDeviceSize size() const noexcept { return this->_size; }

         void* map();
         void unmap();

         void copy_to(buffer&);
   };

   // scene config
   class swap_chain {
      public:
         VkSwapchainKHR handle;
         VkFormat       format;
         VkExtent2D     extent;
         std::vector<VkImage>        images;
         std::vector<VkImageView>    views;
         std::vector<VkFramebuffer>  framebuffers;
         std::vector<VkBuffer>       uniform_buffers;
         std::vector<VkDeviceMemory> uniform_buffer_memory;
         //
         std::vector<VkFence> images_in_flight; // handles. if swap_chain.images[i] is in flight, then swap_chain.images_in_flight[i] == frames_in_flight[x].fence; else, it's a null handle

      public:
         inline size_t image_count() const noexcept { return this->images.size(); }
   };

   struct vertex {
      glm::vec3 pos;
      glm::vec3 color;
      glm::vec2 uv;
      //
      static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
      static VkVertexInputBindingDescription getBindingDescription();
   };

   // a single rendered shape: a vertex buffer and an index buffer, to be used with 
   // shaders to render things to the screen
   class rendered_shape {
      public:
         struct {
            struct {
               buffer   buffer;
               uint32_t indices_at   = 0;
               uint32_t index_count  = 0;
               bool     wide_indices = false;
            } vertex_and_index_buffer;
         } mesh;
         struct {
            VkDescriptorPool             pool;
            std::vector<VkDescriptorSet> sets; // one per frame-in-flight
         } descriptor;
         //
         struct {
            VkDevice         device;
            VkPipeline       pipeline;
            VkPipelineLayout pipeline_layout;
            VkSampler        texture_sampler;
            //
            device_buffer_heap* buffer_heap = nullptr;
            texture*            texture     = nullptr;
         } dependencies;
         //
      public:
         void setBufferHeap(device_buffer_heap*);
         void setDevice(VkDevice);
         void setPipeline(VkPipeline);
         void setPipelineLayout(VkPipelineLayout);
         void setTexture(texture*);
         void setTextureSampler(VkSampler);

         void setMeshData(const std::vector<vertex>&, const std::vector<uint16_t>& indices);
         void setMeshData(const std::vector<vertex>&, const std::vector<uint32_t>& indices);

         void queueDrawing(size_t frame_index, VkCommandBuffer&);

         void setupDescriptorPool(const swap_chain&);
   };

   // a single texture, loaded onto the GPU, for use from a rendered shape's descriptor sets
   struct texture {
      VkImage        image;
      VkDeviceMemory memory;
      VkImageView    view;
   };
}