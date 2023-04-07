#pragma once
#include <vector>
#include "helpers/owned_ptr.h"
#include "./inputs/button.h"
#include "./chrono.h"

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
            not_interrupted,
            interrupted,
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
                  size_t      current_item_index = 0;
                  timestamp_t last_advancement   = zero_timestamp; // for separate-and-ordered groups
               } state;

            public:
               group_update_result update(timestamp_t current_time, devices::abstract_device_handler& device);

               constexpr bool is_ordered() const noexcept {
                  switch (this->type) {
                     case group_type::concurrent_ordered:
                     case group_type::separated_ordered:
                        return true;
                  }
                  return false;
               }

               bool all_contents_inactive() const;
               bool already_consumed() const; // TODO: requires some sort of access to the button states i.e. input device handler
               std::vector<inputs::button> terminal_inputs() const;
               size_t descendant_count() const;
               size_t input_control_count() const;
               bool is_or_contains_input_control(const inputs::button&) const;

               const group* final_group() const;
               group* final_group() {
                  return const_cast<group*>(std::as_const(*this).final_group());
               }

               void find_final_group(const group*& out, const group*& out_parent) const;

               bool operator==(const group&) const;
               bool shallow_equals(const group&) const;
               bool is_superset_of(const group&) const;

            protected:
               void _clear_all_progress();
               std::vector<group*> terminal_items() const;

               group* _clone() const;
         };

      public:
         group* root = nullptr;
         struct {
            enum frame_status frame_status = frame_status::inactive;
            bool frame_status_changed = false;
            timestamp_t went_down_at = zero_timestamp;
         } state;

      public:
         void update(timestamp_t current_time, devices::abstract_device_handler& device);

         bool all_contents_inactive() const;
         void clear_all_progress();

         std::vector<inputs::button> terminal_inputs() const;

         const group* final_group() const;
         group* final_group() {
            return const_cast<group*>(std::as_const(*this).final_group());
         }

         // invoke this on and with absolute input sequences, not relative
         bool is_subset_of(const input_sequence&) const;

         size_t total_group_count() const;
         size_t input_control_count() const;

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
   };
}