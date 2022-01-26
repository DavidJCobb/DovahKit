#pragma once
#include <cstdint>
#include <forward_list>
#include <vector>
#include "_vulkan.h"
#include "_util.h"
#include "surface_renderer.h"

namespace vulkanDK {
   class heap_buffer;
   
   class memory_heap {
      friend class buffer;
      protected:
         struct page;
      
      public:
         static constexpr uint32_t page_size_in_mb = 256;
         static constexpr bool     do_not_share_mappable_pages = true;

         class ownership {
            friend class memory_heap;
            protected:
               memory_heap*       heap = nullptr;
               memory_heap::page* page = nullptr;

               ownership(memory_heap* h, memory_heap::page* p) : heap(h), page(p) {}
            public:
               ownership() {}

               inline surface_renderer* renderer() const noexcept {
                  if (this->heap)
                     return &heap->owner;
                  return nullptr;
               }
               VkDeviceMemory memory() const;

               void teardown(VkDeviceSize offset, VkDeviceSize size);
         };

      protected:
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

         surface_renderer& owner;
         uint32_t          buffer_image_granularity;
         std::vector<pool> pools;

         inline VkDevice logical_device() const { return this->owner.logical_device; }

      public:
         memory_heap(surface_renderer&);
         ~memory_heap() {}

         heap_buffer create_buffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags);

         VkAllocationCallbacks callbacks();
   };
   
   class heap_buffer : no_copy {
      friend class memory_heap;
      protected:
         memory_heap::ownership owner;
         VkDeviceSize _offset = 0;
         VkDeviceSize _size   = 0;

         heap_buffer(memory_heap::ownership, VkDeviceSize offset, VkDeviceSize size);

      public:
         VkBuffer handle = VK_NULL_HANDLE;

         heap_buffer() {}
         ~heap_buffer();

         heap_buffer(heap_buffer&&) noexcept;
         heap_buffer& operator=(heap_buffer&&) noexcept;

         inline bool empty() const noexcept { return this->handle == VK_NULL_HANDLE; }
         inline surface_renderer* renderer() const noexcept { return this->owner.renderer(); }
         inline VkDeviceSize size() const noexcept { return this->_size; }

         void copy_from(const buffer& source);
         void copy_from(const buffer& source, VkDeviceSize);

         void* map_memory(VkDeviceSize offset = 0, VkMemoryMapFlags = 0);
         void* map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags = 0);
         void unmap_memory(void* mapped);
   };
}