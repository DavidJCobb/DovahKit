#pragma once
#include <optional>
#include <vector>
#include <QString>
#include "editor/subsystems/worldedit/enums/editor_mode.h"
#include "./enums/button_press_type.h"
#include "./enums/input_device_type.h"
#include "./input_sequence.h"

namespace dovahkit::subsystems::worldinput2 {
   class combined_tool_results;
}

namespace dovahkit::subsystems::worldinput2 {
   class bind_list_item {
      public:
         QString name;

         std::optional<worldedit::editor_mode> editor_mode;

         button_press_type button_press_type = button_press_type::none;
         input_sequence    input_sequence;

         mutable struct {
            bool press_blocked_hold : 1 = false; // cross-frame Hold node state for Press-preempts-Hold
         } state;

         void invoke(combined_tool_results&) const;
         void invoke_for_hold_release(combined_tool_results&) const;
   };

   class bind_list {
      public:
         input_device_type device_type;

         std::vector<bind_list_item> items;

      protected:
         // If these binds cease to be active on this frame, then we must fire key-up invocations 
         // for them, so they can deactivate their effects as needed.
         std::vector<bind_list_item*> last_frame_active_hold_binds;

      public:
         void update(timestamp_t now, combined_tool_results& press_results, combined_tool_results& hold_results);
   };
}