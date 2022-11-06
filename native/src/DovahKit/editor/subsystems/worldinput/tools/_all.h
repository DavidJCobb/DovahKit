#pragma once
#include <array>
#include <type_traits>
#include "_base.h"
#include "helpers/class_array.h"
#include "../chrono.h"

#include "./attempt_on_screen_selection.h"
#include "./debug_dump_landscape_details.h"
#include "./debug_log.h"
#include "./debug_placeholder.h"
#include "./modify_camera_speed_flags.h"
#include "./move_camera.h"
#include "./turn_camera.h"

namespace dovahkit::subsystems::worldinput {
   using all_tools = cobb::class_array<
      tools::attempt_on_screen_selection,
      tools::debug_dump_landscape_details,
      tools::debug_log,
      tools::debug_placeholder,
      tools::modify_camera_speed_flags,
      tools::move_camera,
      tools::turn_camera//,
   >;

   static_assert(all_tools::size() <= std::numeric_limits<tool_id>::max(), "Worldinput's `tool_id` type must be large enough to have unique values for all entries in `all_tools`, as well as for an additional sentinel \"no tool\" value.");

   template<typename T> concept is_tool = all_tools::contains_type<T>;

   template<typename Options> constexpr tool_id id_of_tool_options() {
      constexpr auto i = all_tools::index_of_matching_type<[]<typename Tool>() {
         if constexpr (tools::tool_has_options_member_type<Tool>) {
            return std::is_same_v<Options, Tool::options>;
         }
         return false;
      }>;
      if (i != (decltype(i))-1)
         return i;
      return tools::id_of_none;
   }
   template<typename T> constexpr tool_id id_of_tool() {
      return all_tools::index_of_type<T>;
   }

   template<typename Options> concept is_tool_options = ([]() { return id_of_tool_options<Options>() != tools::id_of_none; })();

   template<typename T> concept is_tool_or_options = is_tool<T> || is_tool_options<T>;

   class all_tool_instances {
      public:
         static all_tool_instances& get() {
            static all_tool_instances instance;
            return instance;
         }

         using list_type = std::array<tools::base*, all_tools::count>;

      protected:
         all_tool_instances();

         list_type pointers;

      public:
         tool_id id_of(const tools::base& tool) const noexcept;
         const list_type& list() const noexcept { return this->pointers; }

         inline const tools::base* operator[](tool_id i) const {
            if (i == tools::id_of_none)
               return nullptr;
            return this->pointers[i];
         }
   };

   constexpr tool_id id_of_tool(const tools::base* tool) {
      if (!tool)
         return tools::id_of_none;
      return all_tool_instances::get().id_of(*tool);
   }

   // --- tool results ---

   namespace impl {
      template<typename T> struct tool_to_tool_results {
         using type = typename T::results;
      };
   }
   using all_tools_with_results = all_tools::filter_types<[]<typename T>() { return tools::tool_has_results_member_type<T>; }>; // list of all tool classes with results
   using all_tool_results       = all_tools_with_results::map_types<impl::tool_to_tool_results>; // list of all results classes from tools that have them
   
   template<typename Results> requires all_tool_results::contains_type<Results>
   using tool_for_results = all_tools_with_results::nth_type<
      all_tool_results::index_of_type<Results>
   >;

   static_assert(
      !all_tools_with_results::contains_matching_type<
         []<tools::tool_has_results_member_type T>() constexpr {
            return !all_tools_with_results::for_each_until_false<[]<tools::tool_has_results_member_type U>() constexpr {
               return std::is_same_v<T, U> || !std::is_same_v<T::results, U::results>;
            }>();
         }
      >,
      "Different tools cannot have the same results type."
   );
}