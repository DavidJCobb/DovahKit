#pragma once
#include <cstdint>
#include <limits>

struct DKVulkanCameraUpdate;
namespace DK3D {
   struct InputResult;
}

namespace DK3D::editor_functions {
   namespace impl {
      struct option_union_base {
         protected:
            using tag_type = uint8_t;
            static constexpr tag_type empty_tag = std::numeric_limits<tag_type>::max();

         protected:
            const tag_type tag = empty_tag;

            option_union_base(tag_type t) : tag(t) {}

         public:
            inline bool empty() const noexcept { return this->tag == empty_tag; }
            inline static bool empty(const option_union_base& b) { return b.tag == empty_tag; }
      };
   }
   using opaque_option_union = impl::option_union_base;

   class base {
      public:
         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) = 0;
   };
}