#pragma once
#include "./optimize_input_sequence.h"
#include <limits>
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::algorithms {
   constexpr input_sequence_flat optimize_input_sequence(
      const input_sequence& src
   ) {
      using group_type = input_sequence::group_type;

      input_sequence_flat dst;
      if (!src.root)
         return dst;

      size_t total_count = 0;
      size_t raycast_associated_button = std::numeric_limits<size_t>::max();
      {
         auto* rab_ptr = src.raycast.associated_button;
         auto  recurse = [rab_ptr, &raycast_associated_button, &total_count](const input_sequence::group& current, auto& recurse) -> void {
            //
            // - If we're looking for any particular groups, check if `current` is any of them; if so, 
            //   save `total_count` as their index.
            // 
            // - Increment `total_count`.
            // 
            // - Recurse on children.
            //
            if (&current == rab_ptr) {
               assert(current.type == group_type::single_control);
               raycast_associated_button = total_count;
            }
            //
            ++total_count;
            //
            if (current.type == group_type::single_control) {
               assert(current.children.empty());
            } else {
               for (const auto* item : current.children) {
                  assert(item);
                  recurse(*item, recurse);
               }
            }
            return;
         };
         recurse(*src.root, recurse);
      }
      dst.contents.resize(total_count);
      dst.raycast.associated_button = raycast_associated_button;

      {
         size_t count_stored = 0;

         auto add_single_node = [&](const input_sequence::group& subject) {
            size_t dst_index = count_stored;
            auto&  dst_item  = dst.contents[count_stored++];
            //
            dst_item.type = subject.type;
            dst_item.button = subject.button;
            //
            return dst_index;
         };
         auto append_children_of = [&dst, &count_stored, &add_single_node](const input_sequence::group& current, size_t dst_index) -> void {
            auto recurse = [&](const input_sequence::group& current, size_t dst_index, auto& recurse) -> void {
               auto& dst_item = dst.contents[dst_index];

               size_t first_child_index = count_stored;
               dst_item.child_count = current.children.size();
               dst_item.child_ahead = first_child_index - dst_index;

               for (const auto* child : current.children) {
                  add_single_node(*child);
               }

               for (size_t i = 0; i < current.children.size(); ++i) {
                  size_t child_index = i + first_child_index;
                  auto&  child_item  = dst.contents[child_index];
                  recurse(*current.children[i], child_index, recurse);
               }
            };
            return recurse(current, dst_index, recurse);
         };

         auto append_top_level_items = [&add_single_node, &append_children_of]<typename... Types>(Types&... items) -> void {
            (add_single_node(items), ...);

            size_t i = 0;
            (append_children_of(items, i++), ...);
         };
         //
         append_top_level_items(*src.root);
      }

      return dst;
   }
}