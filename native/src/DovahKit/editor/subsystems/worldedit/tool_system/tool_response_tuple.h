#pragma once
#include <bitset>
#include "editor/subsystems/worldinput/chrono.h"
#include "editor/subsystems/worldinput/tool_request_cause.h"
#include "./concepts/is_tool.h"
#include "./concepts/is_tool_response.h"
#include "./concepts/tool_or_tool_response.h"
#include "./concepts/tool_response_or_tool_with_response.h"
#include "./concepts/tool_with_response.h"
#include "./utils/all_tools_with_responses.h"
#include "./utils/all_tool_response_types.h"
#include "./utils/tool_for_response_type.h"
#include "./tools/_all.h"
#include "helpers/array_of_n_values.h"

namespace dovahkit::subsystems::worldedit {
   namespace impl::_tool_response_tuple {
      // ugh, MSVC2019 doesn't support requires expressions...
      template<typename Response> concept can_scale = requires(Response x) { { x.scale(0.0) }; };
      template<typename Response> concept can_merge = requires(Response & x, const Response & y) { { x.merge(std::forward<const Response&>(y)) }; };
   }

   class tool_response_tuple {

      //
      // NOTE: If you ever experience errors where MSVC just completely fucks up all the member 
      // offsets, e.g. accesses to `this->presence` compiling such that they read from and clobber 
      // random tuple members, then clean and rebuild the project. This can happen if MSVC fails 
      // to properly recompile some of the code for this template, while somehow inconsistently 
      // recompiling other code for this template (i.e. outdated members, up-to-date constructor).
      // 
      // If you see issues where all tools suddenly stop working even though they're properly 
      // emitting responses, then check the disassembly for this class's `merge_member` templated 
      // method. Step through until the breakpoint writes ECX for the `std::bitset` method calls, 
      // and then double-check via the Watch panel that ECX is the same as `&this->presence`. If 
      // it isn't, then MSVC fucked up the member offsets.
      //

      private:
         using timestamp_t = worldinput::timestamp_t;
         static constexpr const auto zero_timestamp = worldinput::zero_timestamp;

         using self_t  = tool_response_tuple;
         using classes = tools::all_tool_response_types;

         using response_types_with_timestamps = classes::filter_types<[]<typename Response>() {
            return impl::_tool_response_tuple::can_merge<Response> && (tools::tool_for_response_type<Response>::compile_time_options.use_strict_ordering == true);
         }>;

         template<typename T> requires (tools::all_tools_with_responses::contains_type<T> || tools::is_tool_response<T>)
         using _to_response = std::conditional_t<tools::all_tools_with_responses::contains_type<T>, typename T::response, T>;

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
         // In order to handle those cases, we need to store timestamps for the relevant response 
         // and use these timestamps to apply ordering logic when calling tool::response::merge.
         //
         std::array<timestamp_t, response_types_with_timestamps::count> input_timestamps = cobb::array_of_n_values<response_types_with_timestamps::count>(zero_timestamp);

         std::bitset<tools::all_tools_with_responses::count> presence;

         tools::all_tool_response_types::as_tuple data;

         template<tools::tool_response_or_tool_with_response T>
         static constexpr const size_t _presence_bit_index_of = []() -> size_t {
            if constexpr (tools::is_tool<T>) {
               return tools::all_tools_with_responses::index_of_type<T>;
            } else {
               return tools::all_tool_response_types::index_of_type<T>;
            }
         }();

      public:
         template<tools::tool_response_or_tool_with_response T>
         constexpr bool has_member() const {
            return this->presence.test(_presence_bit_index_of<T>);
         }

         // alternative to std::get which accepts a tool class or a tool::response struct
         template<tools::tool_response_or_tool_with_response T> constexpr const _to_response<T>& get_member() const;
         template<tools::tool_response_or_tool_with_response T> constexpr _to_response<T>& get_member();

         // alternative to std::get-and-then-assign which accepts a tool class or a tool::response struct; 
         // templated type must be specified explicitly
         template<tools::tool_response_or_tool_with_response T>
         void set_member(const _to_response<T>& v);

         // alternative to std::get-and-then-assign which accepts a tool::response struct as an argument
         template<tools::is_tool_response A>
         void set_member(const A& v);

         // Merge a member in without applying any timestamp/ordering logic.
         template<tools::is_tool_response A>
         void merge_member(const A& v);
         
         template<typename A> requires (response_types_with_timestamps::contains_type<A>)
         constexpr timestamp_t& timestamp() {
            return this->input_timestamps[response_types_with_timestamps::index_of_type<A>()];
         }

         template<tools::is_tool_response A>
         void merge_member(timestamp_t time, const A& v);
         
         // More robust version of the above function, capable of handling the release of 
         // a Hold bind.
         template<tools::is_tool_response A>
         void merge_member(const worldinput::tool_request_cause& cause, const A& v);

         void scale(double delta_seconds);
         void merge(const tool_response_tuple& merge_from);
   };
}

#include "./tool_response_tuple.inl"