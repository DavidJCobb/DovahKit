#pragma once
#include <vector>
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

            protected:
               void _clear_all_progress();
               std::vector<group*> terminal_items() const;
         };

      public:
         group* root = nullptr;
         struct {
            enum frame_status frame_status = frame_status::inactive;
            bool frame_status_changed = false;
         } state;

      public:
         void update(timestamp_t current_time, devices::abstract_device_handler& device);

         bool all_contents_inactive() const;
         void clear_all_progress();
   };
}