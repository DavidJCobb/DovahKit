#include "./worldinput2.h"
#include <QApplication>
#include "helpers/unreachable.h"
#include "editor/subsystems/worldedit/tool_system/tool_results_tuple.h"
#include "editor/subsystems/xinput/core.h"
#include "./algorithms/flatten_bind_tree.h"
#include "./bind_tree/nodes/root.h"
#include "./tools/combined_tool_results.h"

#include "./debugging.h"
#include <QLabel>

namespace {
   constexpr float reset_after_lag_threshold = 3.0; // ignore all held inputs if this much time passed since we last polled
}

namespace dovahkit::subsystems::worldinput2 {

   core::core() {
      this->device_handlers.keyboard_mouse.recheck_mouse_metrics();
      QObject::connect((QApplication*)QApplication::instance(), &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
         if (state == Qt::ApplicationState::ApplicationActive) {
            this->device_handlers.keyboard_mouse.recheck_mouse_metrics();
         }
      });
   }
   core::~core() {
   }

   const devices::abstract_device_handler& core::device_by_type(input_device_type type) const {
      switch (type) {
         case input_device_type::keyboard_mouse:
            return this->device_handlers.keyboard_mouse;
         case input_device_type::xinput:
            return this->device_handlers.gamepad;
      }
      cobb::unreachable();
   }
   devices::abstract_device_handler& core::device_by_type(input_device_type type) {
      return const_cast<devices::abstract_device_handler&>(std::as_const(*this).device_by_type(type));
   }

   void core::ignoreAllHeldKeys() {
      this->device_handlers.keyboard_mouse.ignore_all_down();
      this->device_handlers.gamepad.ignore_all_down();
   }
   void core::doPerFrameInputProcessing(double& elapsed_seconds, tool_results_tuple& out) {
      timestamp_t now = current_time();
      elapsed_seconds = elapsed_time(this->state.last_update, now);
      this->state.last_update = now;
      this->state.raycast_results_this_frame = {};
      if (elapsed_seconds <= 0.0) {
         //
         // This can happen sometimes -- we receive  an update so soon that it's not even 
         // an easily-measurable  fraction of a  second -- and in  that case, there's not 
         // any point to doing input processing. We'll just be scaling speeds and whatnot 
         // by zero anyway.
         //
         out = {};
         return;
      }
      if (!this->state.target_widget_has_focus) {
         out = {};
         return;
      }
      //
      // Update input device states:
      //
      this->device_handlers.keyboard_mouse.update(now);
      {
         auto& xinput = subsystems::xinput::core::get();
         xinput.update();
         this->device_handlers.gamepad.update(now, xinput.isGamepadConnected(), xinput.gamepadState());
      }
      //
      // Run the bind tree update algorithm:
      //
      tool_results_tuple press_results;
      tool_results_tuple hold_results;
      this->binds.keyboard.update(now, press_results, hold_results);
      this->binds.gamepad.update(now, press_results, hold_results);
      //
      // The "instant" results are used for "tap" and "hold" button binds, e.g. "tap a key to jump 
      // the camera 8 units to the left." The "while" results are used for "while-down" button 
      // binds and for non-button binds, and are scaled by the frame time, e.g. "hold a key to 
      // move the camera to the left."
      // 
      // As such,...
      // 
      //  - Tool results for a non-"while" button should be considered "instant" results; the tool 
      //    should return values that would make sense to apply instantly.
      // 
      //  - All other results should be considered "timed" results; the tool should return values 
      //    in units per second, as we will here scale them by the frame time in seconds.
      // 
      // This is generally what you'd want. A paintbrush-style tool, for example, would generally 
      // produce as its result an intensity value (e.g. hold the right trigger on a controller to 
      // paint; the amount by which it is held is the intensity of the paint), and for binds that 
      // apply over time, you'd want that to be intensity per second so that changes in frame rate 
      // don't cause the tool to paint unevenly.
      //
      hold_results.scale(elapsed_seconds);
      press_results.merge(hold_results);
      //
      out = press_results;
   }

   void core::setTargetWidget(QWidget* target) {
      auto& current = this->state.target_widget;
      if (current == target)
         return;
      if (current) {
         current->removeEventFilter(this);
      }
      current = target;
      if (target) {
         target->installEventFilter(this);
         this->state.target_widget_has_focus = target->hasFocus();
      } else {
         this->state.target_widget_has_focus = false;
      }
      this->ignoreAllHeldKeys();
   }

   void core::setBindingsFor(input_device_type d, const binds::tree& b) {
      switch (d) {
         using _ = input_device_type;
         case _::keyboard_mouse:
            this->schemes.keyboard = b;
            this->binds.keyboard = algorithms::flatten_bind_tree(this->schemes.keyboard);
            break;
         case _::xinput:
            this->schemes.gamepad = b;
            this->binds.gamepad = algorithms::flatten_bind_tree(this->schemes.gamepad);
            break;
         default:
            return;
      }
   }

   bool core::eventFilter(QObject* watched, QEvent* event) {
      if (watched != this->state.target_widget)
         return false;
      switch (event->type()) {
         case QEvent::Type::FocusIn:
            this->state.target_widget_has_focus = true;
            break;
         case QEvent::Type::FocusOut:
            this->state.target_widget_has_focus = false;
            this->ignoreAllHeldKeys();
            break;
         default:
            return false;
      }
      return false;
   }
}