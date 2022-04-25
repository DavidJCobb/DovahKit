#include "DK3DInputHandler.h"
#include <QApplication>
#include <QMouseEvent>
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "button_state.h"
#include "bind_tree/nodes/input.h"
#include "bind_tree/nodes/root.h"
#include "tools/_results.h"

using namespace DK3D;

namespace {
   constexpr bool test_function_binds = true;
}

namespace {
   constexpr float reset_after_lag_threshold = 3.0; // ignore all held inputs if this much time passed since we last polled

   constexpr double boolean_input_hold_threshold = 0.7;

   template<typename F, typename... T> inline void _for_each(F func, T&... args) {
      (func)(args...);
   };
}

DK3DInputHandler::DK3DInputHandler() {
   this->keyboard_state.recheck_mouse_metrics();
   QObject::connect((QApplication*)QApplication::instance(), &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
      if (state == Qt::ApplicationState::ApplicationActive) {
         this->keyboard_state.recheck_mouse_metrics();
      }
   });
   //
   // Default bindings for testing:
   //
   #pragma region tree binds: keyboard
   {
      auto& tree = this->binds.keyboard;
      auto* root = tree.root;
      //
      auto* mod_ctrl = new binds::nodes::input( // Modifier
         tr("Ctrl Modifier"),
         DK3D::inputs::bound_input{
            .button = {
               .key = cobb::qt::key::from_windows_vk(VK_CONTROL),
               .press_type = DK3D::button_press_type::while_down,
            },
         },
         nullptr,
         true
      );
      root->append(*mod_ctrl);
      //
      {  // keyboard functions: move camera
         struct _bind {
            const char* name;
            char  key;
            float x = 0;
            float y = 0;
            float z = 0;
         };
         constexpr auto _binds = std::array{
            _bind{ ("Move Camera Forward"), 'W',  0,  1,  0 },
            _bind{ ("Move Camera Back"),    'S',  0, -1,  0 },
            _bind{ ("Move Camera Left"),    'A', -1,  0,  0 },
            _bind{ ("Move Camera Right"),   'D',  1,  0,  0 },
            _bind{ ("Move Camera Up"),      'Q',  0,  0,  1 },
            _bind{ ("Move Camera Down"),    'Z',  0,  0, -1 },
         };
         auto* func = &tools::move_camera::get();
         for (const auto& b : _binds) {
            root->append(*(new binds::nodes::input(
               tr(b.name),
               DK3D::inputs::bound_input::from_key(b.key, DK3D::button_press_type::while_down),
               func,
               tools::move_camera::options{
                  .reference_frames = {
                     .baseline  = reference_frame::camera,
                     .selection = reference_frame::camera,
                  },
                  .magnitudes = { .x = b.x, .y = b.y, .z = b.z },
               }
            )));
         }
      }
      {  // keyboard functions: turn camera
         struct _bind {
            const char* name;
            const char  key;
            float x = 0; // pitch
            float z = 0; // yaw
         };
         constexpr auto _binds = std::array{
            _bind{ "Turn Camera Left",  'G',  0, -1}, // clockwise, so turning left is negative
            _bind{ "Turn Camera Right", 'H',  0,  1},
            _bind{ "Turn Camera Up",    'R',  1,  0},
            _bind{ "Turn Camera Down" , 'V', -1,  0},
         };
         auto* func = &tools::turn_camera::get();
         for (const auto& b : _binds) {
            root->append(*(new binds::nodes::input(
               tr(b.name),
               DK3D::inputs::bound_input::from_key(b.key, DK3D::button_press_type::while_down),
               func,
               tools::turn_camera::options{
                  .magnitudes = {.yaw = b.z, .pitch = b.x },
               }
            )));
         }
      }
      //
      root->append(*(new binds::nodes::input(
         tr("Select Object"),
         DK3D::inputs::bound_input::from_mouse_button(Qt::MouseButton::LeftButton, DK3D::button_press_type::tap),
         &tools::attempt_on_screen_selection::get(),
         tools::attempt_on_screen_selection::options{
            .operation = selection_operation::replace,
            .position  = pointer_position_type::mouse,
         }
      )));
      mod_ctrl->append(*(new binds::nodes::input(
         tr("Toggle Selection"),
         DK3D::inputs::bound_input::from_mouse_button(Qt::MouseButton::LeftButton, DK3D::button_press_type::tap),
         &tools::attempt_on_screen_selection::get(),
         tools::attempt_on_screen_selection::options{
            .operation = selection_operation::toggle,
            .position  = pointer_position_type::mouse,
         }
      )));
   }
   #pragma endregion
   #pragma region tree binds: gamepad
   {
      auto& tree = this->binds.gamepad;
      auto* root = tree.root;
      {  // gamepad functions: move camera
         auto* func = &tools::move_camera::get();
         //
         root->append(*(new binds::nodes::input( // Lateral
            tr("Move Camera Laterally"),
            DK3D::inputs::bound_input{
               .vector = {
                  .input = vector_control::xinput_ls,
               },
            },
            func,
            tools::move_camera::options{
               .reference_frames = {
                  .baseline  = reference_frame::camera,
                  .selection = reference_frame::camera,
               },
               .non_button = {
                  .input_x = axis3D::x,
                  .input_y = axis3D::y,
                  .x_sign  = sign::positive,
                  .y_sign  = sign::positive,
               },
            }
         )));
         root->append(*(new binds::nodes::input( // Down
            tr("Move Camera Down"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = DK3D::inputs::xinput_button::LB,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            func,
            tools::move_camera::options{
               .reference_frames = {
                  .baseline  = reference_frame::camera,
                  .selection = reference_frame::camera,
               },
               .magnitudes = { .z = -1 },
            }
         )));
         root->append(*(new binds::nodes::input( // Up
            tr("Move Camera Up"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::RB,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            func,
            tools::move_camera::options{
               .reference_frames = {
                  .baseline  = reference_frame::camera,
                  .selection = reference_frame::camera,
               },
               .magnitudes = { .z = 1 },
            }
         )));
      }
      {  // gamepad functions: turn camera
         root->append(*(new binds::nodes::input(
            tr("Turn Camera"),
            DK3D::inputs::bound_input{
               .vector = {
                  .input = vector_control::xinput_rs,
               },
            },
            &tools::turn_camera::get(),
            tools::turn_camera::options{
               .non_button = {
                  .input_x = camera_turn_axis::yaw,
                  .input_y = camera_turn_axis::pitch,
                  .x_sign  = sign::positive,
                  .y_sign  = sign::negative, // in XInput, up is positive and down is negative; our system presupposes the opposite
               },
            }
         )));
      }
      {  // gamepad functions: boost and precision
         root->append(*(new binds::nodes::input(
            tr("Boost"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = DK3D::inputs::xinput_button::LT,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            &tools::modify_camera_speed_flags::get(),
            tools::modify_camera_speed_flags::options{
               .boost = bool_operation::set_true,
            }
         )));
         root->append(*(new binds::nodes::input(
            tr("Toggle Precision"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = DK3D::inputs::xinput_button::LS,
                  .press_type = DK3D::button_press_type::tap,
               },
            },
            &tools::modify_camera_speed_flags::get(),
            tools::modify_camera_speed_flags::options{
               .precision = bool_operation::invert,
            }
         )));
      }
      //
      // Nested versus non-nested binds:
      //
      {  // Non-nested
         root->append(*(new binds::nodes::input( // Tap
            tr("Test Tap (shadowed)"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::A,
                  .press_type = DK3D::button_press_type::tap,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 1 }
         )));
         root->append(*(new binds::nodes::input( // Hold
            tr("Test Hold (shadowed)"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::B,
                  .press_type = DK3D::button_press_type::hold,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 1 }
         )));
         root->append(*(new binds::nodes::input( // While
            tr("Test While (shadowed)"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::X,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 1 }
         )));
         root->append(*(new binds::nodes::input( // Tap (not shadowed)
            tr("Test Tap (not shadowed)"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::Y,
                  .press_type = DK3D::button_press_type::tap,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 1 }
         )));
      }
      {  // Nested
         auto* node = new binds::nodes::input( // Modifier
            tr("Test Modifier"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::RT,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            nullptr,
            true
         );
         root->append(*node);
         //
         node->append(*(new binds::nodes::input( // Tap
            tr("Move Camera Up"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::A,
                  .press_type = DK3D::button_press_type::tap,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 2 }
         )));
         node->append(*(new binds::nodes::input( // Hold
            tr("Move Camera Up"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::B,
                  .press_type = DK3D::button_press_type::hold,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 2 }
         )));
         node->append(*(new binds::nodes::input( // While
            tr("Move Camera Up"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = inputs::xinput_button::X,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 2 }
         )));
      }
   }
   #pragma endregion
}
DK3DInputHandler::~DK3DInputHandler() {
}

float DK3DInputHandler::scalarControlValue(DK3D::scalar_control c, DK3D::axis2D axis) const {
   switch (c) {
      using _ = DK3D::scalar_control;
      case _::none:
         return 0;
      case _::mouse_move:
         if (axis == axis2D::x)
            return this->keyboard_state.mouse.move.x();
         return this->keyboard_state.mouse.move.y();
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return 0;
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = DK3D::scalar_control;
      case _::xinput_lt:
         return gs.lt;
      case _::xinput_rt:
         return gs.rt;
      case _::xinput_ls:
         if (axis == DK3D::axis2D::x)
            return gs.ls.x();
         return gs.ls.y();
      case _::xinput_rs:
         if (axis == DK3D::axis2D::x)
            return gs.rs.x();
         return gs.rs.y();
   }
   return 0;
}
QPointF DK3DInputHandler::vectorControlValue(DK3D::vector_control c) const {
   switch (c) {
      using _ = DK3D::vector_control;
      case _::none:
         return { 0, 0 };
      case _::mouse_move:
         return this->keyboard_state.mouse.move;
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return { 0, 0 };
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = DK3D::vector_control;
      case _::xinput_ls:
         return gs.ls;
      case _::xinput_rs:
         return gs.rs;
   }
   return { 0, 0 };
}

DK3D::input_result DK3DInputHandler::inputResultOf(const DK3D::inputs::bound_input& input) const {
   DK3D::input_result result;
   if (input.is_button()) {
      bool is_while = (input.button.press_type == DK3D::button_press_type::while_down);
      //
      auto& b = input.button;
      button_state kds;
      if (b.gamepad != DK3D::inputs::xinput_button::None) {
         kds = this->gamepad_state.key_down_state(b);
      } else {
         kds = this->keyboard_state.key_down_state(b);
      }
      auto res = input_result::for_button_input(this->state.last_update, input, kds);
      if (b.mouse != Qt::MouseButton::NoButton) {
         auto pos = this->keyboard_state.mouse.pos;
         if (this->state.target_view) {
            pos = this->state.target_view->mapFromGlobal(pos);
         }
         res.x = pos.x();
         res.y = pos.y();
      }
      return res;
   }
   if (input.is_scalar()) {
      result.type = control_type::scalar;
      result.x = this->scalarControlValue(input.scalar.input, input.scalar.axis);
      return result;
   }
   if (input.is_vector()) {
      result.type = control_type::vector;
      auto xy = this->vectorControlValue(input.vector.input);
      result.x = xy.x();
      result.y = xy.y();
      return result;
   }
   return result;
}

void DK3DInputHandler::ignoreAllHeldKeys(timestamp_t now) {
   if (now == zero_timestamp)
      now = current_time();
   //
   this->keyboard_state.ignore_all_down();
   this->gamepad_state.ignore_all_down();
}
void DK3DInputHandler::updateAllKeys(timestamp_t now) {
   this->keyboard_state.update(now);
   auto& xinput = DKXInputSubsystem::get();
   xinput.update();
   this->gamepad_state.update(now, xinput.isGamepadConnected(), xinput.gamepadState());
}

DK3D::binds::tree DK3DInputHandler::bindingsFor(input_device_type d) const {
   switch (d) {
      using _ = input_device_type;
      case _::keyboard_mouse:
         return this->binds.keyboard;
      case _::xinput:
         return this->binds.gamepad;
   }
   return DK3D::binds::tree(d);
}
void DK3DInputHandler::viewFocusChange(DKVulkanView* target, bool has_focus) {
   if (has_focus) {
      this->setTargetView(target);
   } else {
      if (this->state.target_view == target)
         this->setTargetView(nullptr);
   }
}

void DK3DInputHandler::update(DK3D::combined_tool_results& out, double& elapsed) {
   auto now = DK3D::current_time();
   elapsed = DK3D::elapsed_time(this->state.last_update, now);
   if (elapsed <= 0.0) {
      //
      // This can happen sometimes -- we receive  an update so soon that it's not even 
      // an easily-measurable  fraction of a  second -- and in  that case, there's not 
      // any point to doing input processing. We'll just be scaling speeds and whatnot 
      // by zero anyway.
      //
      out = {};
      return;
   }
   this->state.last_update = now;
   //
   if (elapsed >= reset_after_lag_threshold) {
      this->ignoreAllHeldKeys(now);
      out = {};
      return;
   }
   //
   this->updateAllKeys(now);
   bool has_gamepad = this->gamepad_state.is_connected;
   //
   DK3D::combined_tool_results instant_results;
   DK3D::combined_tool_results while_results;
   this->binds.keyboard.process(instant_results, while_results);
   this->binds.gamepad.process(instant_results, while_results); // process even if no gamepad, so we can handle implicit key-ups
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
   while_results.scale(elapsed);
   instant_results.merge(while_results);
   //
   out = instant_results;
}

void DK3DInputHandler::setTargetView(DKVulkanView* target) {
   if (this->state.target_view == target)
      return;
   this->state.target_view = target;
   this->ignoreAllHeldKeys();
}
DKVulkanCameraUpdate DK3DInputHandler::update(DKVulkanView* subject) {
   if (subject && subject != this->state.target_view)
      return {};
   //
   double elapsed;
   combined_tool_results results;
   this->update(results, elapsed);
   if (elapsed <= 0.0)
      return {};
   //
   DKVulkanCameraUpdate update;
   update.delta_seconds = elapsed;
   {
      const auto& data = results.get_member<DK3D::tools::move_camera>();
      update.move.direction      = { data.x, data.y, data.z };
      update.move.scale_by_delta = false;
      //
      // Results from non-tap binds (e.g. "while" binds, scalars, vectors) get scaled by the 
      // delta in the code above. This means that we need to turn off scaling in this particular 
      // step here.
      //
      update.move.speed = 180.0 * elapsed; // must specify this (as speed * elapsed) rather than relying on direction alone, because the direction vector gets normalized when we pass it in
   }
   {
      const auto& data = results.get_member<DK3D::tools::turn_camera>();
      update.turn.roll  = data.roll;
      update.turn.pitch = data.pitch;
      update.turn.yaw   = data.yaw;
      update.turn.scale_by_delta = false;
      //
      update.turn.speed = glm::radians(90.0F);
   }
   return update;
}

void DK3DInputHandler::setBindingsFor(input_device_type d, const DK3D::binds::tree& b) {
   switch (d) {
      using _ = input_device_type;
      case _::keyboard_mouse:
         this->binds.keyboard = b;
         break;
      case _::xinput:
         this->binds.gamepad = b;
         break;
      default:
         return;
   }
}

bool DK3DInputHandler::_isButtonProcessed(passkey_to_bind_tree, const DK3D::inputs::button& button) const {
   if (button.gamepad != DK3D::inputs::xinput_button::None) {
      return this->gamepad_state.is_button_processed(button);
   }
   return this->keyboard_state.is_button_processed(button);
}
void DK3DInputHandler::_markButtonProcessed(passkey_to_bind_tree, const DK3D::inputs::button& button) {
   if (button.gamepad != DK3D::inputs::xinput_button::None) {
      this->gamepad_state.mark_button_processed(button);
      return;
   }
   this->keyboard_state.mark_button_processed(button);
}