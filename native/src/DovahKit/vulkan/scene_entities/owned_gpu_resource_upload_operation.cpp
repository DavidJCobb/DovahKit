#include "owned_gpu_resource_upload_operation.h"
#include <vector>
#include "../buffer.h"
#include "../surface_renderer.h"

namespace vulkanDK::scene_entities {
   command_buffer& owned_gpu_resource_upload_operation::_get_command_buffer() const {
      return this->owner.uploading.commands;
   }
   buffer& owned_gpu_resource_upload_operation::_get_staging_buffer() const {
      return this->owner.uploading.staging;
   }

   void owned_gpu_resource_upload_operation::stage_data(const void* src, size_t src_size) {
      memcpy(cobb::offset_into(this->staging.data, this->staging.start + this->staging.offset), src, src_size);
      this->staging.size   += src_size;
      this->staging.offset += src_size;
   }

   buffer owned_gpu_resource_upload_operation::create_buffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size) {
      if (size == VK_WHOLE_SIZE) {
         size = this->staging.size;
      }
      return this->owner.create_buffer(size, usage, properties);
   }
   [[nodiscard]] owned_image_and_view owned_gpu_resource_upload_operation::create_image_and_view() {
      return owned_image_and_view(this->owner);
   }
   //
   void owned_gpu_resource_upload_operation::queue_upload_to_buffer(buffer& dst, VkDeviceSize size) {
      if (size == VK_WHOLE_SIZE) {
         size = this->staging.size - this->command_info.offset;
      }
      auto copy_region = VkBufferCopy{
         .srcOffset = this->staging.start,
         .dstOffset = this->command_info.offset,
         .size      = size,
      };
      vkCmdCopyBuffer(_get_command_buffer().handle, _get_staging_buffer().handle, dst.handle, 1, &copy_region);
      this->command_info.offset += size;
   }
   void owned_gpu_resource_upload_operation::queue_upload_to_image(owned_image_and_view& dst, VkImageAspectFlags aspect, VkImageLayout layout, VkAccessFlags access) {
      auto& commands = _get_command_buffer();
      //
      // We don't care what content was present before, so we'll treat the image layout as 
      // undefined and switch to a transfer-destination layout for our copy operation.
      // 
      // Because transfer writes don't need to wait, the source access mask can be empty; 
      // based on that, we'll use the earliest pipeline stage: "top of pipe."
      //
      dst.current.access = 0;
      dst.current.layout = VK_IMAGE_LAYOUT_UNDEFINED;
      dst.transition_layout(commands, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
      //
      std::vector<VkBufferImageCopy> mip_levels;
      //
      {
         std::vector<size_t> mipmap_level_offsets;
         dst.metadata.get_mip_level_offsets(mipmap_level_offsets);
         mip_levels.resize(mipmap_level_offsets.size());
         
         auto base_offset = this->staging.start + this->command_info.offset;
         auto extent = dst.metadata.extent;

         for (size_t i = 0; i < mip_levels.size(); ++i) {
            auto& item = mip_levels[i];
            item = VkBufferImageCopy{
               .bufferOffset      = base_offset + mipmap_level_offsets[i],
               .bufferRowLength   = 0, // amount of padding bytes between rows?
               .bufferImageHeight = 0, // amount of padding bytes... somewhere?
               .imageSubresource  = {
                  .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                  .mipLevel       = (uint32_t)i,
                  .baseArrayLayer = 0,
                  .layerCount     = dst.metadata.layer_count,
               },
               .imageOffset = { 0, 0, 0 },
               .imageExtent = extent,
            };
            extent.width  /= 2;
            extent.height /= 2;
         }
      }
      vkCmdCopyBufferToImage(
         commands.handle,
         _get_staging_buffer().handle,
         dst.handle,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         mip_levels.size(),
         mip_levels.data()
      );
      dst.transition_layout(commands, aspect, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, access);
   }

   void owned_gpu_resource_upload_operation::set_debug_object_name(uint64_t handle, VkObjectType type, const std::string& name) {
      this->owner.set_debug_object_name(handle, type, name);
   }

   void owned_gpu_resource_upload_operation::finish_queueing() {
      _get_staging_buffer().unmap_memory(this->staging.data);
      this->staging.data = nullptr;
      _get_command_buffer().finish();
   }

   void owned_gpu_resource_upload_operation::next(surface_renderer_passkey) {
      this->staging.start += this->staging.size;
      this->staging.offset = 0;
      this->staging.size   = 0;
      this->command_info   = {};
   }
   void owned_gpu_resource_upload_operation::align_to(surface_renderer_passkey, VkDeviceSize align) {
      auto& start = this->staging.start;
      if (auto misalign = start % align; misalign) { // TODO: Use VkPhysicalDeviceLimits::optimalBufferCopyOffsetAlignment
         start += (align - misalign);
      }
   }
}