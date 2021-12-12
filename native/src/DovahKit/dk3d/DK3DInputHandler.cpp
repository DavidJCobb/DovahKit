#include "DK3DInputHandler.h"
#include <QApplication>
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "KeyDownState.h"
#include "bind_tree/nodes/input.h"
#include "bind_tree/nodes/root.h"

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
      auto& tree = this->binds.keyboard_tree;
      auto* root = tree.root;
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
                     .baseline  = ReferenceFrame::Camera,
                     .selection = ReferenceFrame::Camera,
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
            _bind{ "Turn Camera Left",  'G',  0,  1}, // counterclockwise, so turning left is positive
            _bind{ "Turn Camera Right", 'H',  0, -1},
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
   }
   #pragma endregion
   #pragma region tree binds: gamepad
   {
      auto& tree = this->binds.gamepad_tree;
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
                  .baseline  = ReferenceFrame::Camera,
                  .selection = ReferenceFrame::Camera,
               },
               .non_button = {
                  .input_x = Axis3D::X,
                  .input_y = Axis3D::Y,
                  .x_sign  = Sign::Positive,
                  .y_sign  = Sign::Positive,
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
                  .baseline  = ReferenceFrame::Camera,
                  .selection = ReferenceFrame::Camera,
               },
               .magnitudes = { .z = -1 },
            }
         )));
         root->append(*(new binds::nodes::input( // Up
            tr("Move Camera Up"),
            DK3D::inputs::bound_input{
               .button = {
                  .gamepad    = XInputKey::RB,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            func,
            tools::move_camera::options{
               .reference_frames = {
                  .baseline  = ReferenceFrame::Camera,
                  .selection = ReferenceFrame::Camera,
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
                  .input_x = CameraTurnAxis::Yaw,
                  .input_y = CameraTurnAxis::Pitch,
                  .x_sign  = Sign::Positive,
                  .y_sign  = Sign::Positive,
               },
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
                  .gamepad    = XInputKey::A,
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
                  .gamepad    = XInputKey::B,
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
                  .gamepad    = XInputKey::X,
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
                  .gamepad    = XInputKey::Y,
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
                  .gamepad    = XInputKey::LT,
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
                  .gamepad    = XInputKey::A,
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
                  .gamepad    = XInputKey::B,
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
                  .gamepad    = XInputKey::X,
                  .press_type = DK3D::button_press_type::while_down,
               },
            },
            &tools::debug_log::get(),
            tools::debug_log::options{ .number = 2 }
         )));
      }
   }
   #pragma endregion
   #pragma region linear (non-tree) bind lists
   {  // keyboard functions: move camera
      auto& kb = this->binds.keyboard;
      //
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
         kb.push_back(Binding(
            tr(b.name),
            BoundInput::from_key(b.key, BooleanInputMod::While),
            func,
            tools::move_camera::options{
               .reference_frames = {
                  .baseline  = ReferenceFrame::Camera,
                  .selection = ReferenceFrame::Camera,
               },
               .magnitudes = { .x = b.x, .y = b.y, .z = b.z },
            }
         ));
      }
   }
   {  // keyboard functions: turn camera
      auto& kb = this->binds.keyboard;
      //
      struct _bind {
         const char* name;
         const char  key;
         float x = 0; // pitch
         float z = 0; // yaw
      };
      constexpr auto _binds = std::array{
         _bind{ "Turn Camera Left",  'G',  0,  1}, // counterclockwise, so turning left is positive
         _bind{ "Turn Camera Right", 'H',  0, -1},
         _bind{ "Turn Camera Up",    'R',  1,  0},
         _bind{ "Turn Camera Down" , 'V', -1,  0},
      };
      auto* func = &tools::turn_camera::get();
      for (const auto& b : _binds) {
         kb.push_back(Binding(
            tr(b.name),
            BoundInput::from_key(b.key, BooleanInputMod::While),
            func,
            tools::turn_camera::options{
               .magnitudes = {.yaw = b.z, .pitch = b.x },
            }
         ));
      }
   }
   {  // gamepad functions: move camera
      auto& gb   = this->binds.gamepad;
      auto* func = &tools::move_camera::get();
      //
      gb.push_back(Binding( // Lateral
         tr("Move Camera Laterally"),
         BoundInput{
            .vector = {
               .input = VectorControl::XInput_LS,
            },
         },
         func,
         tools::move_camera::options{
            .reference_frames = {
               .baseline  = ReferenceFrame::Camera,
               .selection = ReferenceFrame::Camera,
            },
            .non_button = {
               .input_x = Axis3D::X,
               .input_y = Axis3D::Y,
               .x_sign  = Sign::Positive,
               .y_sign  = Sign::Positive,
            },
         }
      ));
      gb.push_back(Binding( // Down
         tr("Move Camera Down"),
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::LB,
               },
               .type = BooleanInputMod::While,
            },
         },
         func,
         tools::move_camera::options{
            .reference_frames = {
               .baseline  = ReferenceFrame::Camera,
               .selection = ReferenceFrame::Camera,
            },
            .magnitudes = { .z = -1 },
         }
      ));
      gb.push_back(Binding( // Up
         tr("Move Camera Up"),
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::RB,
               },
               .type = BooleanInputMod::While,
            },
         },
         func,
         tools::move_camera::options{
            .reference_frames = {
               .baseline  = ReferenceFrame::Camera,
               .selection = ReferenceFrame::Camera,
            },
            .magnitudes = { .z = 1 },
         }
      ));
   }
   {  // gamepad functions: turn camera
      auto& gb = this->binds.gamepad;
      gb.push_back(Binding(
         tr("Turn Camera"),
         BoundInput{
            .vector = {
               .input = VectorControl::XInput_RS,
            },
         },
         &tools::turn_camera::get(),
         tools::turn_camera::options{
            .non_button = {
               .input_x = CameraTurnAxis::Yaw,
               .input_y = CameraTurnAxis::Pitch,
               .x_sign  = Sign::Positive,
               .y_sign  = Sign::Positive,
            },
         }
      ));
   }
   {  // Gamepad functions: test echo
      auto& gb = this->binds.gamepad;
      gb.push_back(Binding(
         tr("Test Binding (Tap)"),
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::X,
               },
               .type = BooleanInputMod::Tap,
            },
         },
         &tools::debug_log::get(),
         tools::debug_log::options{ .number = 1 }
      ));
      gb.push_back(Binding(
         tr("Test Binding (Hold)"),
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::Y,
               },
               .type = BooleanInputMod::Hold,
            },
         },
         &tools::debug_log::get(),
         tools::debug_log::options{ .number = 2 }
      ));
      gb.push_back(Binding(
         tr("Test Binding (While)"),
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::B,
               },
               .type = BooleanInputMod::While,
            },
         },
         &tools::debug_log::get(),
         tools::debug_log::options{ .number = 3 }
      ));
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

InputResult DK3DInputHandler::inputResultOf(const DK3D::inputs::bound_input& input) const {
   InputResult result;
   if (input.is_button()) {
      bool is_while = (input.button.press_type == DK3D::button_press_type::while_down);
      //
      auto& b = input.button;
      KeyDownState kds;
      if (b.gamepad != DK3D::inputs::xinput_button::None) {
         kds = this->gamepad_state.key_down_state(b);
      } else {
         kds = this->keyboard_state.key_down_state(b);
      }
      return InputResult::for_boolean_input(this->state.last_update, input, kds);
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

QVector<Binding> DK3DInputHandler::bindingsFor(InputDevice d) const {
   switch (d) {
      using _ = InputDevice;
      case _::KeyboardMouse:
         return this->binds.keyboard;
      case _::XInput:
         return this->binds.gamepad;
   }
   return QVector<Binding>();
}
void DK3DInputHandler::viewFocusChange(DKVulkanView* target, bool has_focus) {
   if (has_focus) {
      this->setTargetView(target);
   } else {
      if (this->state.target_view == target)
         this->setTargetView(nullptr);
   }
}

void DK3DInputHandler::setTargetView(DKVulkanView* target) {
   if (this->state.target_view == target)
      return;
   this->state.target_view = target;
   this->ignoreAllHeldKeys();
}
DKVulkanCameraUpdate DK3DInputHandler::update(DKVulkanView* subject) {
   if (subject && subject != this->state.target_view)
      return DKVulkanCameraUpdate();
   //
   auto now     = DK3D::current_time();
   auto elapsed = DK3D::elapsed_time(this->state.last_update, now);
   if (elapsed <= 0.0)
      //
      // This can happen sometimes -- we receive  an update so soon that it's not even 
      // an easily-measurable  fraction of a  second -- and in  that case, there's not 
      // any point to doing input processing. We'll just be scaling speeds and whatnot 
      // by zero anyway.
      //
      return DKVulkanCameraUpdate();
   this->state.last_update = now;
   //
   if (elapsed >= reset_after_lag_threshold) {
      this->ignoreAllHeldKeys(now);
      return DKVulkanCameraUpdate();
   }
   //
   this->updateAllKeys(now);
   bool has_gamepad = this->gamepad_state.is_connected;
   //
   DKVulkanCameraUpdate update;
   update.delta_seconds = elapsed;
   update.move.speed    = 1.0;
   /*//
   for (auto& bind : this->binds.keyboard) {
      if (!bind.function)
         continue;
      auto r = inputResultOf(bind.input);
      if (r.active() || r.while_has_changed) {
         bind.function->invoke(r, bind.params, update);
      }
   }
   if (has_gamepad) {
      for (auto& bind : this->binds.gamepad) {
         if (!bind.function)
            continue;
         auto r = inputResultOf(bind.input);
         if (r.active() || r.while_has_changed) {
            bind.function->invoke(r, bind.params, update);
         }
      }
   }
   //*/
   this->binds.keyboard_tree.process(update);
   this->binds.gamepad_tree.process(update); // process even if no gamepad, so we can handle implicit key-ups

   return update;
}

void DK3DInputHandler::setBindingsFor(InputDevice d, const QVector<Binding>& b) {
   switch (d) {
      using _ = InputDevice;
      case _::KeyboardMouse:
         this->binds.keyboard = b;
         break;
      case _::XInput:
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