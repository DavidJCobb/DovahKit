#pragma once
#include <cstdint>
#include <limits>
#include <type_traits>
#include "../enums/editor_mode.h"

struct DKVulkanCameraUpdate;
namespace dovahkit::subsystems::worldinput {
   class combined_tool_results;
   struct input_result;
}

namespace dovahkit::subsystems::worldinput {
   using tool_id = uint8_t; // unique ID for each tool class, determined automatically at compile-time for everything listed in DK3D::all_tools
}
namespace dovahkit::subsystems::worldinput::tools {
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
   //
   // Base class for tool option unions; this exists so that code which uses an option 
   // union doesn't have to include every single tool option type (which, in the case 
   // of the tools themselves, would require moving the option types to their own files 
   // to avoid cyclical include dependencies).
   // 
   // When you're using the opaque type, you MUST pass it BY REFERENCE, not by value, 
   // to avoid object slicing. Code which receives an opaque union must pointer cast it 
   // to the full union type.
   //
   using opaque_option_union = impl::option_union_base;

   struct compile_time_tool_options {
      bool use_strict_ordering = false; // set to (true) if the order of results within a single frame matters when merging results
   };

   class base {
      protected:
         base() {}

         template<typename T> void setup(T*) {
            this->name = T::function_name;
         }

      public:
         static constexpr compile_time_tool_options compile_time_options = {};
         static constexpr const char* function_name = "unnamed";
         const char* name = function_name;

         virtual void invoke(const input_result&, const opaque_option_union&, combined_tool_results& all_results) const = 0;

         virtual bool has_options() const { return false; }
         virtual editor_mode_set supported_editor_modes() const { return all_editor_modes; }
   };

   template<typename T> concept tool_has_options_member_type = requires { typename T::options; requires std::is_base_of_v<base, T>; };
   template<typename T> concept tool_lacks_options_member_type = !tool_has_options_member_type<T> && std::is_base_of_v<base, T>;

   template<typename T> concept tool_has_results_member_type = requires { typename T::results; requires std::is_base_of_v<base, T>; };
   template<typename T> concept tool_lacks_results_member_type = !tool_has_results_member_type<T> && std::is_base_of_v<base, T>;
}