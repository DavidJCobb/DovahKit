#pragma once
#include <string>
#include "helpers/passkey.h"
#include "../_vulkan.h"
#include "../buffer.h"
#include "../image.h"
#include "../surface_renderer.h"

namespace vulkanDK {
   class command_buffer;
   class surface_renderer;
}

namespace vulkanDK::scene_entities {
   class owned_gpu_resource_upload_operation {
      friend class surface_renderer;
      public:
         using surface_renderer_passkey = cobb::passkey<surface_renderer, owned_gpu_resource_upload_operation>;

      protected:
         surface_renderer& owner;
         size_t       current_entity_index = 0;
         VkDeviceSize max_capacity = VK_WHOLE_SIZE;
         struct {
            void*        data   = nullptr;
            VkDeviceSize start  = 0;
            VkDeviceSize offset = 0;
            VkDeviceSize size   = 0;
         } staging;
         struct {
            VkDeviceSize offset = 0;
            VkDeviceSize size   = 0;
         } command_info;

         command_buffer& _get_command_buffer() const;
         buffer& _get_staging_buffer() const;

      public:
         owned_gpu_resource_upload_operation(surface_renderer& sr) : owner(sr) {}

         void set_max_capacity(surface_renderer_passkey, VkDeviceSize);
         bool has_room_for_more(surface_renderer_passkey) const;

         //
         // An entity should do this first
         //
         void stage_data(const void* src, size_t);

         //
         // Then, it should do these as applicable:
         //
         [[nodiscard]] buffer create_buffer(VkBufferUsageFlags, VkMemoryPropertyFlags, VkDeviceSize size = VK_WHOLE_SIZE);
         [[nodiscard]] owned_image_and_view create_image_and_view();
         //
         void queue_upload_to_buffer(buffer& dst, VkDeviceSize size = VK_WHOLE_SIZE);
         void queue_upload_to_image(owned_image_and_view& dst, VkImageAspectFlags, VkImageLayout, VkAccessFlags);
         //
         void set_debug_object_name(uint64_t handle, VkObjectType type, const std::string& name);
         template<typename T> void set_debug_object_name(T handle, const std::string& name) {
            this->set_debug_object_name((uint64_t)handle, debug_helper_typeof<T>, name);
         }

         //
         // Then, SR moves on to next:
         //
         void next(surface_renderer_passkey);
         void align_to(surface_renderer_passkey, VkDeviceSize);
         void set_entity_index(surface_renderer_passkey, size_t v) { this->current_entity_index = v; }
         size_t get_entity_index() const noexcept { return this->current_entity_index; }

         void finish_queueing();
   };
}