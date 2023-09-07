#pragma once
#include <bitset>
#include "editor/subsystems/worldinput/chrono.h"
#include "editor/subsystems/worldinput/tool_invocation_cause.h"
#include "./concepts/is_tool.h"
#include "./concepts/is_tool_results.h"
#include "./concepts/tool_results_or_tool_with_results.h"
#include "./utils/all_tools_with_results.h"
#include "./utils/all_tool_results.h"
#include "./utils/tool_for_results.h"
#include "./tools/_all.h"
#include "helpers/array_of_n_values.h"

namespace dovahkit::subsystems::worldedit {
   namespace impl::_tool_results_tuple {
      // ugh, MSVC2019 doesn't support requires expressions...
      template<typename Results> concept can_scale = requires(Results x) { { x.scale(0.0) }; };
      template<typename Results> concept can_merge = requires(Results & x, const Results & y) { { x.merge(std::forward<const Results&>(y)) }; };
   }

   // Struct containing one member for each tool's results type.
   class tool_results_tuple : public tools::all_tool_results::as_tuple {
      private:
         using timestamp_t = worldinput::timestamp_t;
         static constexpr const auto zero_timestamp = worldinput::zero_timestamp;

         using self_t  = tool_results_tuple;
         using classes = tools::all_tool_results;

         using result_types_with_timestamps = classes::filter_types<[]<typename Results>() {
            return impl::_tool_results_tuple::can_merge<Results> && (tools::tool_for_results<Results>::compile_time_options.use_strict_ordering == true);
         }>;

         template<typename T> requires (tools::all_tools_with_results::contains_type<T> || tools::is_tool_results<T>)
         using _to_results = std::conditional_t<tools::all_tools_with_results::contains_type<T>, typename T::results, T>;

      public:
         using tuple::tuple; // inherit constructor, etc.

      protected:
         //
         // Some tools are sensitive to activation order within a single frame. Consider, for 
         // example, a tool which sets some enum within the editor state, such that a non-"while" 
         // button should change the enum indefinitely while a "while" button should change the 
         // enum only while the bind is held. What happens if multiple "while" buttons are bound 
         // to the tool, with different options (producing different enum values)? Generally, if 
         // multiple such binds are being held at the same time, the one that went down the most 
         // recently is the one that should take precedence.
         // 
         // In order to handle those cases, we need to store timestamps for the relevant results 
         // and use these timestamps to apply ordering logic when calling tool::results::merge.
         //
         std::array<timestamp_t, result_types_with_timestamps::count> input_timestamps = cobb::array_of_n_values<result_types_with_timestamps::count>(zero_timestamp);

         std::bitset<tools::all_tools_with_results::count> presence;

         template<tools::tool_results_or_tool_with_results T>
         static constexpr const size_t _presence_bit_index_of = []() -> size_t {
            if constexpr (tools::is_tool<T>) {
               return tools::all_tools_with_results::index_of_type<T>;
            } else {
               return tools::all_tool_results::index_of_type<T>;
            }
         }();

      public:

         template<tools::tool_results_or_tool_with_results T>
         constexpr bool has_member() const {
            return this->presence.test(_presence_bit_index_of<T>);
         }

         // alternative to std::get which accepts a tool class or a tool::results struct
         template<tools::tool_results_or_tool_with_results T> constexpr const _to_results<T>& get_member() const;
         template<tools::tool_results_or_tool_with_results T> constexpr _to_results<T>& get_member();

         // alternative to std::get-and-then-assign which accepts a tool class or a tool::results struct; 
         // templated type must be specified explicitly
         template<tools::tool_results_or_tool_with_results T>
         void set_member(const _to_results<T>& v);

         // alternative to std::get-and-then-assign which accepts a tool::results struct as an argument
         template<tools::is_tool_results A>
         void set_member(const A& v);

         // Merge a member in without applying any timestamp/ordering logic.
         template<tools::is_tool_results A>
         void merge_member(const A& v);
         
         template<typename A> requires (result_types_with_timestamps::contains_type<A>)
         constexpr timestamp_t& timestamp() {
            return this->input_timestamps[result_types_with_timestamps::index_of_type<A>()];
         }

         template<tools::is_tool_results A>
         void merge_member(timestamp_t time, const A& v);
         
         // More robust version of the above function, capable of handling the release of 
         // a Hold bind.
         template<tools::is_tool_results A>
         void merge_member(const worldinput::tool_invocation_cause& cause, const A& v);

         void scale(double delta_seconds);
         void merge(const tool_results_tuple& merge_from);
   };
}

#include "./tool_results_tuple.inl"