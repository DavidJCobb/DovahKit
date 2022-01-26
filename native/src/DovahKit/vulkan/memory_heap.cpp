#include "memory_heap.h"
#include <stdexcept>

namespace vulkanDK {
   #pragma region memory_heap
      #pragma region ownership
         VkDeviceMemory memory_heap::ownership::memory() const {
            if (this->page)
               return this->page->memory;
            return VK_NULL_HANDLE;
         }

         void memory_heap::ownership::teardown(VkDeviceSize offset, VkDeviceSize size) {
            if (!this->page)
               return;
            this->page->release(offset);
         }
      #pragma endregion
      #pragma region page
         bool memory_heap::page::request(VkMemoryRequirements requirements, bool linear, uint32_t buffer_image_granularity, VkDeviceSize& offset) {
            auto desired = requirements.size;
            //
            range candidate = { 0, requirements.size, linear };
            auto& list      = this->occupied;
            //
            // Scan through the list and check if the space between any list item is large enough to 
            // hold the desired size. We'll check the space before the current list item.
            //
            std::decay_t<decltype(list)>::iterator back = list.end();
            for (auto it = list.begin(); it != list.end(); back = it, ++it) {
               const auto&  here = *it;
               VkDeviceSize end  = candidate.end();
               if (here.linear != linear)
                  //
                  // Some hardware requires that "linear" and "non-linear" resources have gaps 
                  // placed between them.
                  //
                  end &= ~(VkDeviceSize(buffer_image_granularity) - 1);
               //
               if (end <= here.start) {
                  list.insert_after(it, candidate);
                  offset = candidate.start;
                  return true;
               }
               candidate.start = here.end();
               if (here.linear != linear)
                  candidate.start &= ~(VkDeviceSize(buffer_image_granularity) - 1);
               candidate.start += (candidate.start % requirements.alignment);
            }
            //
            // No space between existing list items. Is there spare space at the end of our memory 
            // region instead?
            //
            if (this->size - candidate.start >= desired) {
               offset = candidate.start;
               if (back != list.end()) {
                  list.insert_after(back, candidate);
               } else {
                  list.push_front(candidate); // empty list
               }
               return true;
            }
            //
            // Hm, looks like we're full. :(
            //
            return false;
         }
         void memory_heap::page::release(VkDeviceSize offset) {
            auto& list = this->occupied;
            //
            std::decay_t<decltype(list)>::iterator prev = list.end();
            for (auto it = list.begin(); it != list.end(); prev = it, ++it) {
               const auto& here = *it;
               if (here.start == offset) {
                  if (prev == list.end()) {
                     assert(it == list.begin());
                     list.pop_front();
                  } else {
                     list.erase_after(prev);
                  }
                  return;
               }
            }
            throw std::runtime_error("[vulkanDK::memory_heap::page::release] Unable to find offset.");
         }
      #pragma endregion
      #pragma region pool
         memory_heap::page& memory_heap::pool::create_page(VkDevice device) {
            auto alloc_info = VkMemoryAllocateInfo{
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .allocationSize  = page_size_in_mb * 1024 * 1024,
               .memoryTypeIndex = this->type_index,
            };
            VkDeviceMemory memory;
            if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
               throw std::runtime_error("[vulkanDK::memory_heap::pool::create_page] Allocation failed.");
            }
         }
      #pragma endregion
      //
      memory_heap::memory_heap(surface_renderer& sr) : owner(sr) {
         this->buffer_image_granularity = sr.device_info->support.memory.buffer_image_granularity;
         //
         VkPhysicalDeviceMemoryProperties properties;
         vkGetPhysicalDeviceMemoryProperties(sr.device_info->handle, &properties);
         //
         for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
            auto& type = properties.memoryTypes[i];
            this->pools.emplace_back(pool{
               .type       = type,
               .type_index = i,
            });
         }
      }
      heap_buffer memory_heap::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
         assert(size < page_size_in_mb * 1024 * 1024);
         //
         VkBuffer buffer_handle;
         auto     buffer_info = VkBufferCreateInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
         };
         if (vkCreateBuffer(this->owner.logical_device, &buffer_info, nullptr, &buffer_handle) != VK_SUCCESS) {
            throw std::runtime_error("[vulkanDK::memory_heap::create_buffer] Failed to create buffer handle.");
         }
         //
         VkMemoryRequirements requirements;
         vkGetBufferMemoryRequirements(this->owner.logical_device, buffer_handle, &requirements);
         //
         for (auto& pool : this->pools) {
            if ((pool.type.propertyFlags & properties) != properties)
               continue;
            //
            // Mappable handling:
            //
            if constexpr (do_not_share_mappable_pages) {
               if (properties != VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                  VkDeviceSize offset;
                  auto& page = pool.create_page(this->owner.logical_device);
                  page.mappable = true;
                  if (page.request(requirements, false, this->buffer_image_granularity, offset))
                     //
                     // Should always succeed, unless the desired size is too large for a page to contain 
                     // (which we check for in an assertion above).
                     //
                     return heap_buffer(ownership{ this, &page }, offset, requirements.size);
                  throw std::runtime_error("[vulkanDK::memory_heap::create_buffer] Failed to create new mappable page.");
               }
            }
            //
            // Normal handling:
            //
            for (auto& page : pool.pages) {
               if constexpr (do_not_share_mappable_pages) {
                  if (page.mappable)
                     continue;
               }
               VkDeviceSize offset;
               if (page.request(requirements, false, this->buffer_image_granularity, offset))
                  return heap_buffer(ownership{ this, &page }, offset, requirements.size);
            }
            //
            // This pool is of the right type, but has insufficient pages or insufficient room within 
            // the extant pages. Try creating a new page.
            //
            VkDeviceSize offset;
            auto& page = pool.create_page(this->owner.logical_device);
            if (page.request(requirements, false, this->buffer_image_granularity, offset))
               //
               // Should always succeed, unless the desired size is too large for a page to contain 
               // (which we check for in an assertion above).
               //
               return heap_buffer(ownership{ this, &page }, offset, requirements.size);
            throw std::runtime_error("[vulkanDK::memory_heap::create_buffer] Failed to create new page.");
         }
         throw std::runtime_error("[vulkanDK::memory_heap::create_buffer] No suitable pool.");
      }
      VkAllocationCallbacks memory_heap::callbacks() {
         return VkAllocationCallbacks{
            .pUserData = this,

         };
      }
   #pragma endregion
   #pragma region heap_buffer
      heap_buffer::heap_buffer(memory_heap::ownership o, VkDeviceSize offset, VkDeviceSize size) : owner(o), _offset(offset), _size(size) {
      }

      heap_buffer::~heap_buffer() {
         this->owner.teardown(this->_offset, this->_size);
      }

      heap_buffer::heap_buffer(heap_buffer&& o) noexcept {
         std::swap(this->owner,   o.owner);
         std::swap(this->_size,   o._size);
         std::swap(this->_offset, o._offset);
      }
      heap_buffer& heap_buffer::operator=(heap_buffer&& o) noexcept {
         std::swap(this->owner,   o.owner);
         std::swap(this->_size,   o._size);
         std::swap(this->_offset, o._offset);
      }

      void heap_buffer::copy_from(const buffer& source) {
         this->renderer()->do_single_commands([this, &source](command_buffer& scratch) {
            auto copy_region = VkBufferCopy{
               .srcOffset = 0,
               .dstOffset = 0,
               .size = source.size,
            };
            vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
         });
      }
      void heap_buffer::copy_from(const buffer& source, VkDeviceSize size) {
         this->renderer()->do_single_commands([this, &source, size](command_buffer& scratch) {
            auto copy_region = VkBufferCopy{
               .srcOffset = 0,
               .dstOffset = 0,
               .size = size,
            };
            vkCmdCopyBuffer(scratch.handle, source.handle, this->handle, 1, &copy_region);
         });
      }

      void* heap_buffer::map_memory(VkDeviceSize offset, VkMemoryMapFlags flags) {
         assert(this->renderer());
         void* data;
         vkMapMemory(this->renderer()->logical_device, this->owner.memory(), offset, this->_size - offset, flags, &data);
         return data;
      }
      void* heap_buffer::map_memory(VkDeviceSize offset, VkDeviceSize length, VkMemoryMapFlags flags) {
         assert(this->renderer());
         void* data;
         vkMapMemory(this->renderer()->logical_device, this->owner.memory(), offset, length, flags, &data);
         return data;
      }
      void heap_buffer::unmap_memory(void* mapped) {
         assert(this->renderer());
         vkUnmapMemory(this->renderer()->logical_device, this->owner.memory());
      }
   #pragma endregion
}