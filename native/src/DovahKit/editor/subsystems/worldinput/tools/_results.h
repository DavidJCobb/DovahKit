#pragma once
#include "./_base.h"
#include "./_all.h"
#include "../chrono.h"
#include "../input_result.h"
#include "helpers/array_of_n_values.h"

namespace dovahkit::subsystems::worldinput {
   class combined_tool_results;
   namespace impl::_combined_tool_results {
      // ugh, MSVC2019 doesn't support requires expressions...
      template<typename Results> concept can_scale = requires(Results x) { { x.scale(0.0) }; };
      template<typename Results> concept can_merge = requires(Results & x, const Results & y) { { x.merge(std::forward<const Results&>(y)) }; };
   }
   // Struct containing one member for each tool's results type.
   class combined_tool_results : public all_tool_results::as_tuple {
      private:
         using self_t  = combined_tool_results;
         using classes = all_tool_results;

         using result_types_with_timestamps = classes::filter_types<[]<typename Results>() {
            return impl::_combined_tool_results::can_merge<Results> && (tool_for_results<Results>::compile_time_options.use_strict_ordering == true);
         }>;

         template<typename T> requires (all_tools_with_results::contains_type<T> || all_tool_results::contains_type<T>)
         using _to_results = std::conditional_t<all_tools_with_results::contains_type<T>, typename T::results, T>;

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

      public:
         // alternative to std::get which accepts a tool class or a tool::results struct
         template<typename T> requires (all_tools_with_results::contains_type<T> || all_tool_results::contains_type<T>)
         _to_results<T>& get_member() {
            return std::get<_to_results<T>>(*this);
         }

         // alternative to std::get-and-then-assign which accepts a tool class or a tool::results struct; 
         // templated type must be specified explicitly
         template<typename T> requires (all_tools_with_results::contains_type<T> || all_tool_results::contains_type<T>)
         void set_member(const _to_results<T>& v) {
            std::get<_to_results<T>>(*this) = v;
         }

         // alternative to std::get-and-then-assign which accepts a tool::results struct as an argument
         template<typename A> requires (all_tool_results::contains_type<A>)
         void set_member(const A& v) {
            std::get<A>(*this) = v;
         }

         // Merge a member in without applying any timestamp/ordering logic.
         template<typename A> requires (all_tool_results::contains_type<A>)
         void merge_member(const A& v) {
            if constexpr (impl::_combined_tool_results::can_merge<A>) {
               std::get<A>(*this).merge(v);
            } else {
               std::get<A>(*this) = v;
            }
         }
         
         template<typename A> requires (result_types_with_timestamps::contains_type<A>)
         timestamp_t& timestamp() {
            return this->input_timestamps[result_types_with_timestamps::index_of_type<A>()];
         }

         template<typename A> requires (all_tool_results::contains_type<A>)
         void merge_member(timestamp_t time, const A& v) {
            if constexpr (result_types_with_timestamps::contains_type<A>) {
               auto& ts     = this->input_timestamps[result_types_with_timestamps::index_of_type<A>()];
               auto& stored = std::get<A>(*this);
               if (ts <= time) {
                  // argument results are newer (e.g. if two "while" binds are held concurrently, prefer the more recently pressed of the two)
                  stored.merge(v);
               } else {
                  // argument results are older
                  auto temp = v;
                  temp.merge(stored);
                  std::swap(stored, temp);
               }
            } else {
               std::get<A>(*this) = v;
            }
         }
         
         // More robust version of the above function, capable of handling the release of 
         // a "while" button bind.
         template<typename A> requires (all_tool_results::contains_type<A>)
         void merge_member(const input_result& ir, const A& v) {
            if constexpr (!impl::_combined_tool_results::can_merge<A>) {
               //
               // For now, results that don't have an explicit merge method also don't 
               // use timestaps or ordering. Maybe we'll change that someday.
               //
               if (ir.is_button_release())
                  return;
               std::get<A>(*this) = v;
               return;
            }
            if constexpr (tool_for_results<A>::compile_time_options.use_strict_ordering == false) {
               //
               // Results that merge, but for tools that don't care about ordering within 
               // a frame (e.g. because the way that they merge results means that order 
               // doesn't matter), should skip all order-related checks.
               //
               this->merge_member(v);
               return;
            }
            //
            // Else, pull the appropriate timestamp from the input_result and then merge 
            // results in chronological order.
            //
            if (ir.is_button_release()) {
               this->merge_member(zero_timestamp, v);
               return;
            }
            if (ir.type == control_type::button && ir.button.press_type == button_press_type::while_down)
               this->merge_member(ir.button.down_when, v);
            else
               this->merge_member(v);
         }

         void scale(double delta_seconds) {
            classes::for_each<[]<typename T>(self_t* self, const double delta_seconds) {
               if constexpr (impl::_combined_tool_results::can_scale<T>) {
                  std::get<T>(*self).scale(delta_seconds);
               }
            }>(this, delta_seconds);
         }
         void merge(const combined_tool_results& merge_from) {
            classes::for_each<[]<typename T>(self_t* self, const self_t& source) {
               if constexpr (impl::_combined_tool_results::can_merge<T>) {
                  std::get<T>(*self).merge(std::get<T>(source));
               }
            }>(this, merge_from);
         }
   };
}