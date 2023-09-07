#pragma once
#include <cstdint>
#include <limits> // std::numeric_limits
#include <type_traits>
#include <utility> // std::as_const
#include <vector>
#include "helpers/flat_tree_interface/node_base.h"
#include "./chrono.h"
#include "./input_sequence.h"
#include "./interruption_check.h"

namespace dovahkit::subsystems::worldinput2 {
   class input_sequence_flat;
}
namespace dovahkit::subsystems::worldinput2::algorithms {
   constexpr input_sequence_flat optimize_input_sequence(const input_sequence& src);
}

namespace dovahkit::subsystems::worldinput2 {
   class input_sequence_flat {
      public:
         class group;

      public:
         using group_type = input_sequence::group_type;
         using index_type = uint16_t;
         //
         struct group_update_params {
            timestamp_t         current_time;
            devices::abstract_device_handler& device;
            interruption_check& interruption_check;
            timestamp_t         last_advancement_time;
            struct {
               bool already_passed = false;
               //
               const group* associated_button = nullptr;
               const raycast_requirement& requirement;
            } raycast;
         };
         using group_update_result = input_sequence::group_update_result;

         using frame_status      = input_sequence::frame_status;
         using raycast_status    = input_sequence::raycast_status;
         using range_requirement = input_sequence::range_requirement;

         static constexpr const size_t index_of_none = std::numeric_limits<size_t>::max();

      public:
         class group;

         class group : public cobb::flat_tree_interface::node_base<group> {
            friend class input_sequence_flat;
            friend constexpr input_sequence_flat dovahkit::subsystems::worldinput2::algorithms::optimize_input_sequence(const input_sequence& src);
            public:
               group_type type = group_type::single_control;

               inputs::button button; // for single controls only
            protected:
               struct {
                  size_t current_item_index = 0;
               } state;

            public:
               constexpr bool can_have_children() const noexcept;
               constexpr bool is_concurrent() const noexcept;
               constexpr bool is_ordered() const noexcept;

               constexpr const group* final_group() const;
               constexpr group* final_group() { return const_cast<group*>(std::as_const(*this).final_group()); }
               //
               constexpr void find_final_group(const group*& out, const group*& out_parent) const;
               constexpr void find_final_group(group*& out, group*& out_parent) {
                  const group* a;
                  const group* b;
                  std::as_const(*this).find_final_group(a, b);
                  out = const_cast<group*>(a);
                  out_parent = const_cast<group*>(b);
               }

               constexpr void terminal_inputs(std::vector<inputs::button>& append_to) const;

               // only meaningful when called on a separate-and-ordered group; result is undefined otherwise
               constexpr const group& current_item() const;

            #pragma region Algorithms
            protected:
               constexpr bool _is_separate_ordered_group_complete(interruption_check&) const;
            public:
               constexpr bool run_interruption_check(interruption_check&) const;
               
               group_update_result update(
                  const group_update_params,
                  timestamp_t previous_sibling_time = zero_timestamp
               );

               constexpr void clear_all_progress();
            #pragma endregion
         };

      public:
         constexpr input_sequence_flat() {}

         std::vector<group> contents;
         range_requirement range;
         struct {
            size_t associated_button = index_of_none;
            raycast_requirement requirement;
         } raycast;
         struct {
            typename frame_status frame_status = frame_status::inactive;
            bool frame_status_changed = false;
            timestamp_t last_advancement = zero_timestamp; // timestamp at which the user entered the next key in this input sequence
            timestamp_t went_down_at     = zero_timestamp;
            //
            bool raycast_success_flag = false;
         } state;

      public:
         constexpr const group* root() const;
         constexpr group* root() { return const_cast<group*>(std::as_const(*this).root()); }

         constexpr bool has_any_buttons() const;
         constexpr bool has_range_requirement() const;
         constexpr bool has_raycast_requirement() const;
         constexpr bool is_probably_keyboard_impossible() const;
         constexpr size_t specificity() const;
         constexpr std::vector<inputs::button> terminal_inputs() const;

         constexpr const group* final_group() const;
         constexpr group* final_group() { return const_cast<group*>(std::as_const(*this).final_group()); }

         #pragma region Algorithms
         void update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check&);

         constexpr void clear_all_progress();
         #pragma endregion
   };
}

#include "./input_sequence_flat.inl"