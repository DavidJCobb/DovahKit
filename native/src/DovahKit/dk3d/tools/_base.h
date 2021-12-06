#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>

class  DK3DBindingOptionsContainerWidget;
struct DKVulkanCameraUpdate;
namespace DK3D {
   struct InputResult;
}

namespace DK3D {
   using tool_id = uint8_t; // unique ID for each tool class, determined automatically at compile-time for everything listed in DK3D::all_tools
}
namespace DK3D::tools {
   inline constexpr tool_id id_of_none = std::numeric_limits<tool_id>::max();

   namespace impl {
      struct option_union_base {
         protected:
            tool_id tag = id_of_none;

            option_union_base(tool_id t) : tag(t) {}

         public:
            inline bool empty() const noexcept { return this->tag == id_of_none; }
            inline static bool empty(const option_union_base& b) { return b.tag == id_of_none; }

            inline tool_id id() const noexcept { return this->tag; }
      };
   }
   using opaque_option_union = impl::option_union_base;

   class base {
      protected:
         base() {}

         template<typename T> void setup(T*) {
            this->name = T::function_name;
         }

      public:
         static constexpr const char* function_name = "unnamed";
         const char* name = function_name;

         virtual void invoke(const InputResult&, const opaque_option_union&, DKVulkanCameraUpdate& camera_update) const = 0;

         virtual bool has_options() const { return false; }
   };

   template<typename T> concept tool_has_options_member_type = requires { typename T::options; requires std::is_base_of_v<base, T>; };
   template<typename T> concept tool_lacks_options_member_type = !tool_has_options_member_type<T> && std::is_base_of_v<base, T>;
}