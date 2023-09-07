#pragma once
#include <cstdint>
#include <utility> // std::pair
#include <vector>
#include "helpers/owned_ptr.h"
#include "./enums/axis2D.h"
#include "./enums/range_input_axes.h"
#include "./enums/range_input_control.h"
#include "./inputs/button.h"
#include "./chrono.h"
#include "./interruption_check.h"
#include "./raycast_requirement.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}
namespace cobb::streams {
   class bitreader;
   class bitwriter;
}
namespace dovahkit::subsystems::worldinput::devices {
   class abstract_device_handler;
}

namespace dovahkit::subsystems::worldinput {
   class input_sequence {
      public:
         class group;

         static constexpr const uint32_t serialization_version = 0;

      public:
         enum class frame_status {
            inactive,
            down,
            released,
         };

         enum class raycast_status {
            unaffected,
            passed,
            failed,
         };

         enum class group_type {
            single_control,
            concurrent_ordered,
            concurrent_unordered,
            separated_ordered,
         };

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

         struct group_update_result {
            timestamp_t    down_at    = zero_timestamp;
            size_t         down_count = 0;
            frame_status   status     = frame_status::inactive;
            raycast_status raycast    = raycast_status::unaffected;
         };
         
         struct range_requirement {
            range_input_control control = range_input_control::none;
            range_input_axes    axes    = range_input_axes::all;

            constexpr bool operator==(const range_requirement& other) const = default;

            bool is_satisfied(const devices::abstract_device_handler&) const;

            constexpr void stream(cobb::bitstreams::reader&);
            constexpr void stream(cobb::bitstreams::writer&) const;
         };

         class group {
            friend class input_sequence;
            public:
               constexpr group() {}
               constexpr ~group();

               group_type type = group_type::single_control;

               inputs::button button; // for single controls only
               std::vector<group*> children; // except for single controls

               constexpr group(const group&) = delete;
               constexpr group(group&&) noexcept = delete;
               constexpr group& operator=(const group&) const = delete;
               constexpr group& operator=(group&&) const noexcept = delete;

            protected:
               struct {
                  size_t current_item_index = 0;
               } state;

            public:
               group_update_result update(
                  const group_update_params,
                  timestamp_t previous_sibling_time = zero_timestamp
               );

            protected:
               bool _is_separate_ordered_group_complete(interruption_check&) const;
            public:
               bool run_interruption_check(interruption_check&) const;

               constexpr bool can_have_children() const noexcept;
               constexpr bool is_concurrent() const noexcept;
               constexpr bool is_ordered() const noexcept;

               constexpr void terminal_inputs(std::vector<inputs::button>& append_to) const;
               constexpr size_t input_control_count() const;
               constexpr bool is_or_contains_input_control(const inputs::button&) const;

               // only meaningful when called on a separate-and-ordered group; result is undefined otherwise
               constexpr const group& current_item() const;

               const group* final_group() const;
               group* final_group() {
                  return const_cast<group*>(std::as_const(*this).final_group());
               }

               void find_final_group(const group*& out, const group*& out_parent) const;
               void find_final_group(group*& out, group*& out_parent) {
                  const group* a;
                  const group* b;
                  std::as_const(*this).find_final_group(a, b);
                  out        = const_cast<group*>(a);
                  out_parent = const_cast<group*>(b);
               }

               bool operator==(const group&) const;

               void normalize(bool recursively = false);

               constexpr void stream(cobb::bitstreams::reader&);
               constexpr void stream(cobb::bitstreams::writer&) const;

            protected:
               void _clear_all_progress();

               group* _clone() const;
         };

      public:
         group* root = nullptr; // owned pointer
         range_requirement range;
         struct {
            group* associated_button = nullptr; // unowned pointer (points to `root` or something inside of `root`)
            raycast_requirement requirement;
         } raycast;
         struct {
            enum frame_status frame_status = frame_status::inactive;
            bool frame_status_changed = false;
            timestamp_t last_advancement = zero_timestamp; // timestamp at which the user entered the next key in this input sequence
            timestamp_t went_down_at     = zero_timestamp;
            //
            bool raycast_success_flag = false;
         } state;

      public:
         constexpr input_sequence() {}
         input_sequence(const input_sequence& o) { *this = o; }
         constexpr input_sequence(input_sequence&& o) noexcept { *this = std::move(o); }
         constexpr ~input_sequence();

         void update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check&);

         void clear_all_progress();

         constexpr bool has_any_buttons() const;
         constexpr bool has_range_requirement() const;
         constexpr bool has_raycast_requirement() const;
         constexpr bool is_probably_keyboard_impossible() const;
         constexpr size_t specificity() const;
         constexpr std::vector<inputs::button> terminal_inputs() const;

         const group* final_group() const;
         group* final_group() {
            return const_cast<group*>(std::as_const(*this).final_group());
         }

         input_sequence clone() const; // does not clone run-time-only state

         bool operator==(const input_sequence& other) const {
            if (this->root && other.root)
               return *(this->root) == *(other.root);
            if (this->root != other.root)
               return false;
            return this->range == other.range;
         }

         input_sequence& operator=(const input_sequence& other);
         constexpr input_sequence& operator=(input_sequence&& other) noexcept;

         // absolute = modifier << nested;
         input_sequence operator<<(const input_sequence& nested) const;

         // absolute = modifier.clone();
         // absolute <<= nested;
         input_sequence& operator<<=(const input_sequence& nested);

         // May set `root` to `nullptr` if it's an empty non-single-button ISG.
         void normalize();

         constexpr void stream(cobb::bitstreams::reader&);
         constexpr void stream(cobb::bitstreams::writer&) const;
   };
}

#include "./input_sequence.inl"