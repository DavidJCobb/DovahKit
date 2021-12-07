#include "DK3DInputHandler.h"
#include <QApplication>
#include "../editor/subsystems/DKXInputSubsystem.h"
#include "KeyDownState.h"

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
   this->keyboard_state.recheckMouseMetrics();
   QObject::connect((QApplication*)QApplication::instance(), &QApplication::applicationStateChanged, this, [this](Qt::ApplicationState state) {
      if (state == Qt::ApplicationState::ApplicationActive) {
         this->keyboard_state.recheckMouseMetrics();
      }
   });
   //
   // Default bindings for testing:
   //
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
}
DK3DInputHandler::~DK3DInputHandler() {
}

float DK3DInputHandler::scalarControlValue(ScalarControl c, Axis2D axis) const {
   switch (c) {
      using _ = ScalarControl;
      case _::None:
         return 0;
      case _::MouseMove:
         if (axis == Axis2D::X)
            return this->keyboard_state.mouse.move.x();
         return this->keyboard_state.mouse.move.y();
      case _::MouseWheel:
         return 0; // TODO
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return 0;
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = ScalarControl;
      case _::XInput_LT:
         return gs.lt;
      case _::XInput_RT:
         return gs.rt;
      case _::XInput_LS:
         if (axis == Axis2D::X)
            return gs.ls.x();
         return gs.ls.y();
      case _::XInput_RS:
         if (axis == Axis2D::X)
            return gs.rs.x();
         return gs.rs.y();
   }
   return 0;
}
QPointF DK3DInputHandler::vectorControlValue(VectorControl c) const {
   switch (c) {
      using _ = VectorControl;
      case _::None:
         return { 0, 0 };
      case _::MouseMove:
         return this->keyboard_state.mouse.move;
   }
   auto& xi = DKXInputSubsystem::get();
   if (!xi.isGamepadConnected())
      return { 0, 0 };
   auto& gs = xi.gamepadState();
   switch (c) {
      using _ = VectorControl;
      case _::XInput_LS:
         return gs.ls;
      case _::XInput_RS:
         return gs.rs;
   }
   return { 0, 0 };
}

InputResult DK3DInputHandler::inputResultOf(const BoundInput& input) const {
   InputResult result;
   if (input.is_boolean()) {
      bool is_while = (input.boolean.type == BooleanInputMod::While);
      //
      auto& b = input.boolean;
      KeyDownState kds;
      if (b.mouse.button != Qt::MouseButton::NoButton) {
         kds = this->keyboard_state.keyDownState(b.mouse.button);
      } else if (b.gamepad.button != XInputKey::None) {
         kds = this->gamepad_state.keyDownState(b.gamepad.button);
      } else {
         kds = this->keyboard_state.keyDownState(b.key.native.vk);
      }
      return InputResult::for_boolean_input(this->state.last_update, input, kds);
   }
   if (input.is_scalar()) {
      result.type = InputResult::Type::Scalar;
      result.x = this->scalarControlValue(input.scalar.input, input.scalar.axis);
      return result;
   }
   if (input.is_vector()) {
      result.type = InputResult::Type::Vector;
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
   this->keyboard_state.ignoreAllDown();
   this->gamepad_state.ignoreAllDown();
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

#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include "widgets/DKBoundInputWidget.h"
void DK3DInputHandler::debugOpenBindEditWindow() {
   auto* dialog = new QDialog;
   auto* layout = new QGridLayout(dialog);
   auto* scroll = new QScrollArea(dialog);
   QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
   layout->addWidget(scroll, 0, 0, 1, 2);
   //
   auto* body = new QWidget(scroll);
   {
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, body);
      //
      layout->addWidget(new QLabel("KEYBOARD:", body));
      {
         auto& list = this->binds.keyboard;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto& bind = list[i];
            if (bind.function) {
               layout->addWidget(new QLabel(bind.function->name, body));
            } else {
               layout->addWidget(new QLabel("no function", body));
            }
            auto* t = new DKBoundInputWidget(body);
            t->setProperty("DK3D-input-index", i);
            t->setInputDevice(DKBoundInputWidget::InputDevice::KeyboardMouse);
            t->setValue(bind.input);
            layout->addWidget(t);
         }
      }
      layout->addWidget(new QLabel("GAMEPAD:", body));
      {
         auto& list = this->binds.gamepad;
         auto  size = list.size();
         for (size_t i = 0; i < size; ++i) {
            auto& bind = list[i];
            if (bind.function) {
               layout->addWidget(new QLabel(bind.function->name, body));
            } else {
               layout->addWidget(new QLabel("no function", body));
            }
            auto* t = new DKBoundInputWidget(body);
            t->setProperty("DK3D-input-index", i);
            t->setInputDevice(DKBoundInputWidget::InputDevice::XInput);
            t->setValue(bind.input);
            layout->addWidget(t);
         }
      }
      scroll->setWidget(body);
   }
   auto* save = new QPushButton("Save", dialog);
   QObject::connect(save, &QPushButton::clicked, dialog, [this, dialog, body]() {
      this->ignoreAllHeldKeys();
      //
      auto kids = body->findChildren<DKBoundInputWidget*>();
      for (auto* widget : kids) {
         auto idx = widget->property("DK3D-input-index");
         if (!idx.isValid())
            continue;
         auto i = idx.toInt();
         //
         switch (widget->inputDevice()) {
            using _ = DKBoundInputWidget::InputDevice;
            case _::KeyboardMouse:
               this->binds.keyboard[i].input = widget->value();
               break;
            case _::XInput:
               this->binds.gamepad[i].input = widget->value();
               break;
         }
      }
      //
      dialog->accept();
   });
   layout->addWidget(save, 1, 1);
   layout->addWidget(new DKBoundInputWidget(dialog), 2, 0, 1, 2); // TEST
   //
   dialog->show();
}