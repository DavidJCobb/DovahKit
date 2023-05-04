#pragma once
#include <concepts>
#include <type_traits>
#include "helpers/tuples/contains_type_matching_functor.h"
#include "editor/subsystems/worldinput2/tool_invocation_cause.h"

#include "../all_tools.h"

#include "./attempt_on_screen_selection.h"
#include "./debug_dump_landscape_details.h"
#include "./debug_print.h"
#include "./modify_camera_speed_flags.h"
#include "./move_camera.h"
#include "./turn_camera.h"

namespace dovahkit::subsystems::worldedit {
   class opaque_options_union;
   class tool_results_tuple;
}

namespace dovahkit::subsystems::worldedit::tools {
   template<typename Tool> concept is_valid_tool = requires {
      requires std::is_base_of_v<_base, Tool>;

      // Must override the name.
      requires Tool::function_name != _base::function_name;

      // Tool must be invocable.
      requires requires(const worldinput2::tool_invocation_cause& a, const opaque_options_union& b, tool_results_tuple& c) {
         { Tool::invoke(a, b, c) } -> std::same_as<void>;
      };
      requires requires(const opaque_options_union& b, tool_results_tuple& c) {
         { Tool::invoke_for_hold_release(b, c) } -> std::same_as<void>;
      };
   };

   static_assert(
      []() constexpr -> bool {
         return all_tools::for_each_until_false<[]<typename Current>() -> bool {
            return is_valid_tool<Current>;
         }>();
      }(),
      "All tools must meet the requirements indicated here."
   );

   // Enforce: no two tools can have the same results type.
   static_assert(
      []() constexpr -> bool {
         bool any_two_same = all_tools::for_each_until_true<[]<typename A>() -> bool {
            if constexpr (requires { typename A::results; }) {
               return all_tools::for_each_until_true<[]<typename B>() -> bool {
                  if constexpr (std::is_same_v<A, B>) {
                     return false;
                  }
                  if constexpr (requires { typename B::results; }) {
                     return std::is_same_v<typename A::results, typename B::results>;
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
}