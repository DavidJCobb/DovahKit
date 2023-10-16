#pragma once
#include <concepts>
#include <type_traits>
#include "helpers/tuples/contains_type_matching_functor.h"
#include "editor/subsystems/worldinput/tool_request_cause.h"

#include "../concepts/tool_with_options.h"
#include "../concepts/tool_with_response.h"
#include "../all_tools.h"

#include "./attempt_on_screen_selection.h"
#include "./debug_dump_landscape_details.h"
#include "./debug_dump_raycast.h"
#include "./debug_print.h"
#include "./modify_camera_speed_flags.h"
#include "./move_camera.h"
#include "./move_selection.h"
#include "./move_selection_by_drag.h"
#include "./orbit_camera.h"
#include "./set_edit_gizmo_mode.h"
#include "./turn_camera.h"

namespace dovahkit::subsystems::worldedit {
   class opaque_options_union;
   class tool_response_tuple;
}

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Tool> concept is_valid_tool = requires {
      requires std::is_base_of_v<_base, Tool>;

      // Must override the name.
      requires Tool::function_name != _base::function_name;

      // Tool must be invocable.
      requires requires(const worldinput::tool_request_cause& a, const opaque_options_union& b, tool_response_tuple& c) {
         { Tool::request(a, b, c) } -> std::same_as<void>;
      };
      requires requires(const opaque_options_union& b, tool_response_tuple& c) {
         { Tool::request_for_hold_release(b, c) } -> std::same_as<void>;
      };

      // Tool must have a valid ID to use for serialization.
      { Tool::function_code } -> std::same_as<const cobb::eight_cc&>;
      requires (Tool::function_code != cobb::eight_cc(0));
   };

   static_assert(
      []() constexpr -> bool {
         return all_tools::for_each_until_false<[]<typename Current>() -> bool {
            return is_valid_tool<Current>;
         }>();
      }(),
      "All tools must meet the requirements indicated here."
   );

   // Enforce: no two tools can have the same serialization code.
   static_assert(
      []() constexpr -> bool {
         constexpr bool any_two_same = all_tools::for_each_until_true<[]<typename A>() -> bool {
            return all_tools::for_each_until_true<[]<typename B>() -> bool {
               if constexpr (std::is_same_v<A, B>) {
                  return false;
               }
               return A::function_code == B::function_code;
            }>();
         }>();
         return !any_two_same;
      }(),
      "No two tools can have the same serialization code."
   );


   // Enforce: no two tools can have the same results type.
   static_assert(
      []() constexpr -> bool {
         constexpr bool any_two_same = all_tools::template for_each_until_true<[]<typename A>() -> bool {
            if constexpr (tool_with_response<A>) {
               return all_tools::for_each_until_true<[]<typename B>() -> bool {
                  if constexpr (std::is_same_v<A, B>) {
                     return false;
                  }
                  if constexpr (tool_with_response<B>) {
                     return std::is_same_v<typename A::response, typename B::response>;
                  }
                  return false;
               }>();
            }
            return false;
         }>();
         return !any_two_same;
      }(),
      "No two tools can have the same results type."
   );
   
   // Enforce: raycast-sensitive tools must require strict ordering, and cannot have mergeable results.
   //
   // This is for one simple reason: if the raycast target or hit position is relevant to the tool's 
   // operation, then how do you "merge" two separate raycasts? Remember: some tools react to the 
   // target of a raycast made when the triggering bind originally went down, sometimes even if the 
   // tool itself is only activated when the bind is released; and so multiple invocations of the 
   // same tool could be reacting to different raycast results. The only way to cope with this is to 
   // mandate that no merging occur: we react to only one raycast at a time, preferring a consistent 
   // and predictable order (i.e. timestamps) rather than the implementation-defined order in which 
   // binds are processed.
   //
   static_assert(
      []() constexpr -> bool {
         constexpr bool valid = all_tools::for_each_until_true<[]<typename A>() -> bool {
            if constexpr (A::is_raycast_sensitive) {
               if constexpr (tool_with_response<A>) {
                  using response = typename A::response;
                  if constexpr (requires(response & a, const response & b) { a.merge(b); }) {
                     // Fail: Responses are mergeable.
                     return false;
                  }
               }
               if (!A::compile_time_options.use_strict_ordering) {
                  // Fail: Strict ordering not required.
                  return false;
               }
            }
            return true;
         }>();
         return valid;
      }(),
      "Raycast-sensitive tools must require strict ordering, and cannot have mergeable results."
   );
}