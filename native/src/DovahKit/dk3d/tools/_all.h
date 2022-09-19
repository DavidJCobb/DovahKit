#pragma once
#include <array>
#include <type_traits>
#include "_base.h"
#include "../../helpers/class_array.h"
#include "../chrono.h"

#include "attempt_on_screen_selection.h"
#include "debug_log.h"
#include "debug_placeholder.h"
#include "modify_camera_speed_flags.h"
#include "move_camera.h"
#include "turn_camera.h"

namespace DK3D {
   using all_tools = cobb::class_array<
      tools::attempt_on_screen_selection,
      tools::debug_log,
      tools::debug_placeholder,
      tools::modify_camera_speed_flags,
      tools::move_camera,
      tools::turn_camera//,
   >;

   static_assert(all_tools::size() <= std::numeric_limits<tool_id>::max(), "The DK3D::tool_id type must be large enough to have unique values for all entries in DK3D::all_tools, as well as a sentinel \"no tool\" value.");

   namespace impl {
      template<typename Desired> struct id_of_tool_by_options {
         //
         // We need to be able to use cobb::class_list::index_of_matching to find 
         // the editor_function class that has a given options type. However, the 
         // functors we pass to index_of_matching can only be templated on the 
         // type currently being iterated... so we'll use nested structs along 
         // with nested templating. If, given some type T, you want to find the 
         // editor function class whose options type is T, then you'd want to 
         // pass (id_of_tool_by_options<T>::functor) to index_of_matching.
         //
         template<typename Current> struct functor {
            static consteval bool execute() {
               if constexpr (tools::tool_has_options_member_type<Current>) {
                  return std::is_same_v<Desired, Current::options>;
               }
               return false;
            }
         };
      };
   }
   template<typename Options> consteval tool_id id_of_tool_options() {
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
   template<typename T> consteval tool_id id_of_tool() {
      auto i = all_tools::index_of_type<T>;
      if (i != (decltype(i))-1)
         return i;
      return id_of_tool_options<T>();
   }

   template<typename Options> inline constexpr bool is_tool_options = ([]() { return id_of_tool_options<Options>() != tools::id_of_none; })();

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