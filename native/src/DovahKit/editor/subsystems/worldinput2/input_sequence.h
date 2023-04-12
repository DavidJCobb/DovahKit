#pragma once
#include <utility> // std::pair
#include <vector>
#include "helpers/owned_ptr.h"
#include "./inputs/button.h"
#include "./chrono.h"
#include "./interruption_check.h"

namespace dovahkit::subsystems::worldinput2::devices {
   class abstract_device_handler;
}

namespace dovahkit::subsystems::worldinput2 {
   class input_sequence {
      public:
         enum class frame_status {
            inactive,
            down,
            released,
         };

         enum class group_type {
            single_control,
            concurrent_ordered,
            concurrent_unordered,
            separated_ordered,
         };

         enum class group_update_result {
            no_change,

            // It was determined that the user's attempt to enter the input sequence was interrupted 
            // somehow. Perhaps they released one of the keys in a concurrent group before entering 
            // the group's contents in full, for example.
            interrupted,

            // The user is advancing within the input sequence; the next key that they need to press 
            // is down, or was just released.
            advancing,
         };

         class group {
            friend class input_sequence;
            public:
               group_type type = group_type::single_control;

               inputs::button button; // for single controls only
               std::vector<group*> children; // except for single controls

               ~group();

            protected:
               struct {
                  enum frame_status frame_status = frame_status::inactive;
                  bool frame_status_changed = false;
                  //
                  size_t current_item_index = 0;
               } state;

            public:
               group_update_result update(timestamp_t current_time, timestamp_t last_advancement_time, devices::abstract_device_handler& device);

               void run_interruption_check(interruption_check&) const;

               constexpr bool is_concurrent() const noexcept {
                  switch (this->type) {
                     case group_type::concurrent_ordered:
                     case group_type::concurrent_unordered:
                        return true;
                  }
                  return false;
               }
               constexpr bool is_ordered() const noexcept {
                  switch (this->type) {
                     case group_type::concurrent_ordered:
                     case group_type::separated_ordered:
                        return true;
                  }
                  return false;
               }

               bool all_contents_inactive() const;
               void terminal_inputs(std::vector<inputs::button>& append_to) const;
               size_t input_control_count() const;
               bool is_or_contains_input_control(const inputs::button&) const;

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
               bool shallow_equals(const group&) const;
               bool is_superset_of(const group&) const;

               void debug_stringify(std::string&) const;

            protected:
               void _clear_all_progress();

               group* _clone() const;
         };

      public:
         group* root = nullptr;
         struct {
            enum frame_status frame_status = frame_status::inactive;
            bool frame_status_changed = false;
            timestamp_t last_advancement = zero_timestamp; // timestamp at which the user entered the next key in this input sequence
            timestamp_t went_down_at     = zero_timestamp;
         } state;

      public:
         void update(timestamp_t current_time, devices::abstract_device_handler& device, interruption_check&);

         bool run_interruption_check(interruption_check&) const;

         bool all_contents_inactive() const;
         void clear_all_progress();

         std::vector<inputs::button> terminal_inputs() const;

         const group* final_group() const;
         group* final_group() {
            return const_cast<group*>(std::as_const(*this).final_group());
         }

         // invoke this on and with absolute input sequences, not relative
         bool is_subset_of(const input_sequence&) const;

         size_t specificity() const;

         input_sequence clone() const; // does not clone run-time-only state

         bool operator==(const input_sequence& other) const {
            if (this->root && other.root)
               return *(this->root) == *(other.root);
            return this->root == other.root;
         }

         // absolute = modifier << nested;
         input_sequence operator<<(const input_sequence& nested) const;

         // absolute = modifier.clone();
         // absolute <<= nested;
         input_sequence& operator<<=(const input_sequence& nested);

         void debug_stringify(std::string&) const;
         static input_sequence debug_from_string(const std::string&, bool gamepad = false);
   };
}