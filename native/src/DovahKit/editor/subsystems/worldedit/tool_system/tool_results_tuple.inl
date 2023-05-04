#pragma once
#include "./tool_results_tuple.h"

namespace dovahkit::subsystems::worldedit {
   template<tools::tool_results_or_tool_with_results T>
   constexpr const tool_results_tuple::_to_results<T>& tool_results_tuple::get_member() const {
      return std::get<_to_results<T>>(*this);
   }

   template<tools::tool_results_or_tool_with_results T>
   constexpr tool_results_tuple::_to_results<T>& tool_results_tuple::get_member() {
      return const_cast<_to_results<T>&>(std::as_const(*this).get_member<T>());
   }
   
   template<tools::tool_results_or_tool_with_results T>
   void tool_results_tuple::set_member(const _to_results<T>& v) {
      std::get<_to_results<T>>(*this) = v;
      this->presence.set(_presence_bit_index_of<T>);
   }

   template<tools::is_tool_results A>
   void tool_results_tuple::set_member(const A& v) {
      std::get<A>(*this) = v;
      this->presence.set(_presence_bit_index_of<A>);
   }
   
   template<tools::is_tool_results A>
   void tool_results_tuple::merge_member(const A& v) {
      if constexpr (impl::_tool_results_tuple::can_merge<A>) {
         if (this->presence.test(_presence_bit_index_of<A>)) {
            std::get<A>(*this).merge(v);
            return;
         }
      }
      std::get<A>(*this) = v;
      this->presence.set(_presence_bit_index_of<A>);
   }

   template<tools::is_tool_results A>
   void tool_results_tuple::merge_member(timestamp_t time, const A& v) {
      if constexpr (result_types_with_timestamps::contains_type<A> && impl::_tool_results_tuple::can_merge<A>) {
         auto& ts     = this->input_timestamps[result_types_with_timestamps::index_of_type<A>()];
         auto& stored = std::get<A>(*this);
         if (this->presence.test(_presence_bit_index_of<A>)) {
            if (ts <= time) {
               // argument results are newer (e.g. if two "while" binds are held concurrently, prefer the more recently pressed of the two)
               stored.merge(v);
               ts = time;
            } else {
               // argument results are older
               auto temp = v;
               temp.merge(stored);
               std::swap(stored, temp);
            }
         } else {
            this->presence.set(_presence_bit_index_of<A>);
            stored = v;
            ts     = time;
         }
         return;
      }
      this->presence.set(_presence_bit_index_of<A>);
      std::get<A>(*this) = v;
   }
   
   template<tools::is_tool_results A>
   void tool_results_tuple::merge_member(const worldinput2::tool_invocation_cause& cause, const A& v) {
      if constexpr (!impl::_tool_results_tuple::can_merge<A>) {
         //
         // For now, results that don't have an explicit merge method also don't 
         // use timestaps or ordering. Maybe we'll change that someday.
         //
         this->presence.set(_presence_bit_index_of<A>);
         std::get<A>(*this) = v;
         return;
      }
      if constexpr (tools::tool_for_results<A>::compile_time_options.use_strict_ordering == false) {
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
      if (cause.has_button) {
         if (!cause.button.is_down) {
            this->merge_member(zero_timestamp, v);
            return;
         }
         if (cause.button.press_type == worldinput2::button_press_type::hold) {
            this->merge_member(cause.button.down_when, v);
            return;
         }
      }
      this->merge_member(v);
   }
}