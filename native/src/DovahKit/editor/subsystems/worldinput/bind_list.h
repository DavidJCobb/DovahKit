#pragma once
#include <optional>
#include <vector>
#include <QPointF>
#include <QString>
#include "editor/subsystems/worldedit/enums/editor_mode.h"
#include "editor/subsystems/worldedit/tool_system/opaque_options_union.h"
#include "editor/subsystems/worldedit/tool_system/tool_id.h"
#include "./enums/button_press_type.h"
#include "./enums/input_device_type.h"
#include "./enums/range_input_axes.h"
#include "./util/range_control_conflict_state.h"
#include "./util/tool_binding.h"
#include "./condition_set.h"
#include "./input_sequence.h"

namespace dovahkit::subsystems::worldedit {
   class tool_response_tuple;
}
namespace dovahkit::subsystems::worldinput {
   struct tool_request_cause;
}

namespace dovahkit::subsystems::worldinput {
   class bind_list_item {
      public:
         bind_list_item() {}
         bind_list_item(const bind_list_item&);
         bind_list_item(bind_list_item&&) noexcept;
         ~bind_list_item();

         QString name;
         //
         std::optional<condition_set> conditions;
         //
         button_press_type button_press_type = button_press_type::none;
         input_sequence    input_sequence;
         //
         util::tool_binding bound_tool;

         mutable struct {
            // cross-frame Hold node state for Press-preempts-Hold
            bool press_blocked_hold : 1 = false;

            // mid-frame conflict resolution state for range inputs
            util::range_control_conflict_state range_conflicted_axes;
         } state;

         bind_list_item& operator=(const bind_list_item&);
         bind_list_item& operator=(bind_list_item&&) noexcept;

         // Pass `pos` and `pos_is_delta` if the associated input sequence has a directional constraint; pass whatever's associated with the specified scalar or vector input control.
         // For a scalar input control, the value is always in pos.x(), even if the scalar is generated from e.g. the vertical axis of a vector.
         void invoke(worldedit::tool_response_tuple&, const tool_request_cause&) const;
         void invoke_for_hold_release(worldedit::tool_response_tuple&) const;
   };

   class bind_list {
      public:
         bind_list(input_device_type t) : device_type(t) {}

         input_device_type device_type;

         std::vector<bind_list_item> items;

      protected:
         // If these binds cease to be active on this frame, then we must fire key-up invocations 
         // for them, so they can deactivate their effects as needed.
         std::vector<bind_list_item*> last_frame_active_hold_binds;

      public:
         void update(timestamp_t now, worldedit::tool_response_tuple& press_results, worldedit::tool_response_tuple& hold_results);
   };
}