#include "rendered_shape.h"
#include <stdexcept>

namespace DovahKit::vulkan {
   #pragma region independent_buffer
   independent_buffer::independent_buffer(VkDevice d) : device(d) {}
   independent_buffer::~independent_buffer() {
      if (!this->handle)
         return;
      vkDestroyBuffer(this->device, this->handle, nullptr);
      vkFreeMemory   (this->device, this->memory, nullptr);
   }
   independent_buffer::independent_buffer(independent_buffer&& o) {
      this->device = o.device;
      std::swap(this->handle, o.handle);
      std::swap(this->memory, o.memory);
   }
   independent_buffer& independent_buffer::operator=(independent_buffer&& o) {
      std::swap(this->device, o.device);
      std::swap(this->handle, o.handle);
      std::swap(this->memory, o.memory);
      return *this;
   }
   #pragma endregion

   #pragma region device_buffer_heap
      #pragma region page
         bool device_buffer_heap::page::request(VkMemoryRequirements requirements, bool linear, uint32_t buffer_image_granularity, VkDeviceSize& offset) {
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
         void device_buffer_heap::page::release(VkDeviceSize offset) {
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
            throw std::runtime_error("[vulkan::device_buffer_heap::page::release] Unable to find offset.");
         }
      #pragma endregion
      #pragma region pool
         device_buffer_heap::page& device_buffer_heap::pool::create_page(VkDevice device) {
            auto alloc_info = VkMemoryAllocateInfo{
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .allocationSize  = page_size_in_mb * 1024 * 1024,
               .memoryTypeIndex = this->type_index,
            };
            VkDeviceMemory memory;
            if (vkAllocateMemory(device, &alloc_info, nullptr, &memory) != VK_SUCCESS) {
               throw std::runtime_error("[vulkan::device_buffer_heap::pool::create_page] Allocation failed.");
            }
         }
      #pragma endregion
      //
      device_buffer_heap::device_buffer_heap(VkDevice d, VkPhysicalDevice pd, uint32_t big) : device(d), buffer_image_granularity(big) {
         VkPhysicalDeviceMemoryProperties properties;
         vkGetPhysicalDeviceMemoryProperties(pd, &properties);
         //
         for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
            auto& type = properties.memoryTypes[i];
            this->pools.emplace_back(pool{
               .type       = type,
               .type_index = i,
            });
         }
      }
      buffer device_buffer_heap::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
         assert(size < page_size_in_mb * 1024 * 1024);
         //
         VkBuffer buffer_handle;
         auto     buffer_info = VkBufferCreateInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
         };
         if (vkCreateBuffer(this->device, &buffer_info, nullptr, &buffer_handle) != VK_SUCCESS) {
            throw std::runtime_error("[vulkan::device_buffer_heap::create_buffer] Failed to create buffer handle.");
         }
         //
         VkMemoryRequirements requirements;
         vkGetBufferMemoryRequirements(this->device, buffer_handle, &requirements);
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
                  auto& page = pool.create_page(this->device);
                  page.mappable = true;
                  if (page.request(requirements, false, this->buffer_image_granularity, offset))
                     //
                     // Should always succeed, unless the desired size is too large for a page to contain 
                     // (which we check for in an assertion above).
                     //
                     return buffer(*this, page, offset, requirements.size);
                  throw std::runtime_error("[vulkan::device_buffer_heap::create_buffer] Failed to create new mappable page.");
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
                  return buffer(*this, page, offset, requirements.size);
            }
            //
            // This pool is of the right type, but has insufficient pages or insufficient room within 
            // the extant pages. Try creating a new page.
            //
            VkDeviceSize offset;
            auto& page = pool.create_page(this->device);
            if (page.request(requirements, false, this->buffer_image_granularity, offset))
               //
               // Should always succeed, unless the desired size is too large for a page to contain 
               // (which we check for in an assertion above).
               //
               return buffer(*this, page, offset);
            throw std::runtime_error("[vulkan::device_buffer_heap::create_buffer] Failed to create new page.");
         }
         throw std::runtime_error("[vulkan::device_buffer_heap::create_buffer] No suitable pool.");
      }
      VkAllocationCallbacks device_buffer_heap::callbacks() {
         return VkAllocationCallbacks{
            .pUserData = this,

         };
      }
   #pragma endregion
   #pragma region buffer
      buffer::buffer(device_buffer_heap& h, device_buffer_heap::page& p, VkDeviceSize o, VkDeviceSize size) : owner(&h), page(&p), _offset(o), _size(size) {
      }

      buffer::~buffer() {
         if (this->page)
            this->page->release(this->_offset);
      }

      buffer::buffer(buffer&& o) noexcept : owner(o.owner), page(o.page), _offset(o._offset), _size(o._size) {
         o.owner = nullptr;
         o.page  = nullptr;
      }
      buffer& buffer::operator=(buffer&& o) noexcept {
         std::swap(this->owner,   o.owner);
         std::swap(this->page,    o.page);
         std::swap(this->_offset, o._offset);
         std::swap(this->_size,   o._size);
      }

      void* buffer::map() {
         void* data;
         vkMapMemory(this->owner->device, this->page->memory, this->_offset, this->_size, 0, &data);
         return data;
      }
      void buffer::unmap() {
         vkUnmapMemory(this->owner->device, this->page->memory);
      }
      void buffer::copy_to(buffer& target) {
         assert(target._size >= this->_size);
         VkCommandBuffer command_buffer;
         {
            auto alloc_info = VkCommandBufferAllocateInfo{
               .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
               .commandPool        = this->command_pool,
               .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
               .commandBufferCount = 1,
            };
            VkCommandBuffer single_time_command_buffer;
            vkAllocateCommandBuffers(this->owner->device, &alloc_info, &single_time_command_buffer);
            //
            auto begin_info = VkCommandBufferBeginInfo{
               .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
               .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            };
            vkBeginCommandBuffer(single_time_command_buffer, &begin_info);
         }
         //
         auto copy_region = VkBufferCopy{
            .size = this->_size,
         };
         vkCmdCopyBuffer(command_buffer, this->handle, target.handle, 1, &copy_region);
         //
         {
            vkEndCommandBuffer(command_buffer);
            //
            auto submit_info = VkSubmitInfo{
               .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
               .commandBufferCount = 1,
               .pCommandBuffers    = &command_buffer,
            };
            vkQueueSubmit(this->queues.graphics, 1, &submit_info, VK_NULL_HANDLE);
            vkQueueWaitIdle(this->queues.graphics);
            //
            vkFreeCommandBuffers(this->owner->device, this->command_pool, 1, &command_buffer);
         }
      }
   #pragma endregion

   void rendered_shape::setMeshData(const std::vector<vertex>& verts, const std::vector<uint16_t>& indices) {
      static_assert(false, "TODO: Ensure that the shape isn't currently being drawn.");
      //
      auto& dep = this->dependencies;
      auto& vib = this->mesh.vertex_and_index_buffer;
      //
      VkDeviceSize buffer_size_v = sizeof(vertex) * verts.size();
      VkDeviceSize buffer_size_i = sizeof(std::decay_t<decltype(indices)>::value_type) * indices.size();
      //
      VkDeviceSize buffer_size = buffer_size_v + buffer_size_i;
      //
      // Our normal vertex buffer is going to have a flag set which renders its contents entirely 
      // inaccessible to the CPU; this aids in performance. But how, then, shall we get our vertices 
      // from the CPU to the GPU? We'll use a staging buffer -- a temporary GPU-side buffer which 
      // lacks this flag.
      //
      auto staging = dep.buffer_heap->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      //
      // We want to transfer our vertex data to the GPU. We'll do this by mapping a section of 
      // CPU-accessible memory, copying the data into that section, and then unmapping it.
      //
      void* data = staging.map();
      memcpy(data, verts.data(),   buffer_size_v);
      memcpy(data, indices.data(), buffer_size_i);
      staging.unmap();
      //
      // Now let's create our normal buffer, and transfer data from the staging buffer to the 
      // normal buffer.
      //
      if (!vib.buffer.exists()) {
         vib.buffer = dep.buffer_heap->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      }
      static_assert(false, "TODO: Recreate the VIB buffer if it needs to be resized.");
      staging.copy_to(vib.buffer);
   }
   void rendered_shape::setMeshData(const std::vector<vertex>& verts, const std::vector<uint32_t>& indices) {
      static_assert(false, "TODO: Ensure that the shape isn't currently being drawn.");
      //
      auto& dep = this->dependencies;
      auto& vib = this->mesh.vertex_and_index_buffer;
      //
      VkDeviceSize buffer_size_v = sizeof(vertex) * verts.size();
      VkDeviceSize buffer_size_i = sizeof(std::decay_t<decltype(indices)>::value_type) * indices.size();
      //
      VkDeviceSize buffer_size = buffer_size_v + buffer_size_i;
      //
      auto staging = dep.buffer_heap->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      //
      void* data = staging.map();
      memcpy(data, verts.data(),   buffer_size_v);
      memcpy(data, indices.data(), buffer_size_i);
      staging.unmap();
      //
      vib.wide_indices = true;
      if (!vib.buffer.exists()) {
         vib.buffer = dep.buffer_heap->create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      }
      static_assert(false, "TODO: Recreate the VIB buffer if it needs to be resized.");
      staging.copy_to(vib.buffer);
   }

   void rendered_shape::queueDrawing(size_t frame_index, VkCommandBuffer& command_buffer) {
      auto& dep = this->dependencies;
      auto& vib = this->mesh.vertex_and_index_buffer;
      vkCmdBindVertexBuffers (command_buffer, 0, vib.indices_at, &vib.buffer.handle, 0);
      vkCmdBindIndexBuffer   (command_buffer, vib.buffer.handle, vib.indices_at, vib.wide_indices ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
      vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dep.pipeline_layout, 0, 1, &this->descriptor.sets[frame_index], 0, nullptr);
      //
      vkCmdDrawIndexed(command_buffer, vib.index_count, 1, vib.indices_at, 0, 0);
   }
}