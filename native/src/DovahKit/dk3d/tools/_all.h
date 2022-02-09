#pragma once
#include <array>
#include "_base.h"
#include "../../helpers/class_list.h"
#include "../chrono.h"

#include "debug_log.h"
#include "debug_placeholder.h"
#include "move_camera.h"
#include "turn_camera.h"

namespace DK3D {
   using all_tools = cobb::class_list<
      tools::debug_log,
      tools::debug_placeholder,
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
      constexpr auto i = all_tools::index_of_matching([]<typename Tool>() {
         if constexpr (tools::tool_has_options_member_type<Tool>) {
            return std::is_same_v<Options, Tool::options>;
         }
         return false;
      });
      if (i != (decltype(i))-1)
         return i;
      return tools::id_of_none;
   }
   template<typename T> consteval tool_id id_of_tool() {
      auto i = all_tools::index_of<T>();
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
         using type = T::results;
      };
   }
   using all_tools_with_results = all_tools::all_matching<[]<typename T>() { return tools::tool_has_results_member_type<T>; }>; // list of all tool classes with results
   using all_tool_results       = all_tools_with_results::transform<impl::tool_to_tool_results>; // list of all results classes from tools that have them
   
   #pragma region tool_for_results<Results>
   template<typename Results> struct tool_for_results_f {
      template<typename Tool> struct functor {
         static constexpr bool execute() {
            return std::is_same_v<Results, Tool::results>;
         }
      };
      using result = all_tools_with_results::get_matching<functor>;
   };
   template<typename Results> using tool_for_results = tool_for_results_f<Results>::result; // workaround MSVC2019 bug with the below code: internal compiler error
   /*
   template<typename Results> struct tool_for_results_s {
      using type = all_tools_with_results::get_matching<[]<typename Tool>() constexpr {
         return std::is_same_v<Results, Tool::results>;
      }>;
   };
   template<typename Results> using tool_for_results = tool_for_results_s<Results>::type; // workaround MSVC2019 bug with the below code: templated lambdas do not work with templated aliases; fixed in MSVC2022
   */
   /*
   template<typename Results> using tool_for_results = all_tools_with_results::get_matching<[]<typename Tool>() {
      return std::is_same_v<Results, Tool::results>;
   }>;
   */
   #pragma endregion

   static_assert(
      !all_tools_with_results::has_matching([]<tools::tool_has_results_member_type T>() constexpr {
         return all_tools_with_results::for_each_breakable([]<tools::tool_has_results_member_type U>() constexpr {
            return !std::is_same_v<T, U> && std::is_same_v<T::results, U::results>;
         });
      }),
      "Different tools cannot have the same results type."
   );
}