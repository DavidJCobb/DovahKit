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
   {
      auto& kb = this->binds.keyboard.list;
      //
      struct _bind {
         const char key;
         float x = 0;
         float y = 0;
         float z = 0;
      };
      constexpr auto _binds = std::array{
         _bind{ 'W',  0,  1,  0 },
         _bind{ 'S',  0, -1,  0 },
         _bind{ 'A', -1,  0,  0 },
         _bind{ 'D',  1,  0,  0 },
         _bind{ 'Q',  0,  0,  1 },
         _bind{ 'Z',  0,  0, -1 },
      };
      auto* func = &editor_functions::move_camera::get();
      for (const auto& b : _binds) {
         kb.push_back(Binding{
            BoundInput::from_key(b.key, BooleanInputMod::While),
            func,
            editor_functions::move_camera::options{
               .reference_frames = {
                  .baseline  = ReferenceFrame::Camera,
                  .selection = ReferenceFrame::Camera,
               },
               .magnitudes = { .x = b.x, .y = b.y, .z = b.z },
            }
         });
      }
   }
   {  // gamepad functions: move camera
      auto& gb   = this->binds.gamepad.list;
      auto* func = &editor_functions::move_camera::get();
      //
      gb.push_back(Binding{ // Lateral
         BoundInput{
            .vector = {
               .input = VectorControl::XInput_LS,
            },
         },
         func,
         editor_functions::move_camera::options{
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
      });
      gb.push_back(Binding{ // Down
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::LB,
               },
               .type = BooleanInputMod::While,
            },
         },
         func,
         editor_functions::move_camera::options{
            .reference_frames = {
               .baseline  = ReferenceFrame::Camera,
               .selection = ReferenceFrame::Camera,
            },
            .magnitudes = { .z = -1 },
         }
      });
      gb.push_back(Binding{ // Up
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::RB,
               },
               .type = BooleanInputMod::While,
            },
         },
         func,
         editor_functions::move_camera::options{
            .reference_frames = {
               .baseline  = ReferenceFrame::Camera,
               .selection = ReferenceFrame::Camera,
            },
            .magnitudes = { .z = 1 },
         }
      });
   }
   {  // Gamepad functions: test echo
      auto& gb = this->binds.gamepad.list;
      gb.push_back(Binding{
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::X,
               },
               .type = BooleanInputMod::Tap,
            },
         },
         &editor_functions::debug_log::get(),
         editor_functions::debug_log::options{ .number = 1 }
      });
      gb.push_back(Binding{
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::Y,
               },
               .type = BooleanInputMod::Hold,
            },
         },
         &editor_functions::debug_log::get(),
         editor_functions::debug_log::options{ .number = 2 }
      });
      gb.push_back(Binding{
         BoundInput{
            .boolean = {
               .gamepad = {
                  .button = XInputKey::B,
               },
               .type = BooleanInputMod::While,
            },
         },
         &editor_functions::debug_log::get(),
         editor_functions::debug_log::options{ .number = 3 }
      });
   }
   //
   // Hardcoded bindings for testing:
   //
   if constexpr (test_function_binds) {
      auto& cb = this->binds.keyboard.camera.move;
      cb.forward = BoundInput::from_key('W', BooleanInputMod::While);
      cb.back    = BoundInput::from_key('S', BooleanInputMod::While);
      cb.left    = BoundInput::from_key('A', BooleanInputMod::While);
      cb.right   = BoundInput::from_key('D', BooleanInputMod::While);
      cb.up      = BoundInput::from_key('Q', BooleanInputMod::While);
      cb.down    = BoundInput::from_key('Z', BooleanInputMod::While);
   }
   {
      auto& ct = this->binds.keyboard.camera.turn;
      ct.left  = BoundInput::from_key('G', BooleanInputMod::While);
      ct.right = BoundInput::from_key('H', BooleanInputMod::While);
      ct.up    = BoundInput::from_key('R', BooleanInputMod::While);
      ct.down  = BoundInput::from_key('V', BooleanInputMod::While);
   }
   if constexpr (test_function_binds) {
      auto& cb = this->binds.gamepad.camera.move;
      cb.lateral.vector.input = VectorControl::XInput_LS;
      cb.down = BoundInput::from_xinput_button(DK3D::XInputKey::LB, BooleanInputMod::While);
      cb.up = BoundInput::from_xinput_button(DK3D::XInputKey::RB, BooleanInputMod::While);
   }
   {
      auto& cb = this->binds.gamepad.camera.turn;
      cb.yaw.scalar.input = ScalarControl::XInput_RS;
      cb.yaw.scalar.axis  = Axis2D::X;
      cb.pitch.scalar.input = ScalarControl::XInput_RS;
      cb.pitch.scalar.axis  = Axis2D::Y;
   }
   //
   if constexpr (test_function_binds) { // Test binds:
      auto& cb = this->binds.gamepad;
      cb.test_tap   = BoundInput::from_xinput_button(DK3D::XInputKey::X, BooleanInputMod::Tap);
      cb.test_hold  = BoundInput::from_xinput_button(DK3D::XInputKey::Y, BooleanInputMod::Hold);
      cb.test_while = BoundInput::from_xinput_button(DK3D::XInputKey::B, BooleanInputMod::While);
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
            return this->state.mousemove.x;
         return this->state.mousemove.y;
      case _::MouseWheel:
         return this->state.mousemove.wheel;
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
         return { (qreal)this->state.mousemove.x, (qreal)this->state.mousemove.y };
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
   if (has_gamepad) {
      auto& gp = this->binds.gamepad;
      if (inputResultOf(gp.test_tap).active()) {
         qDebug("[DK3DInputHandler] \"Tap\" test bind activated.");
      }
      if (inputResultOf(gp.test_hold).active()) {
         qDebug("[DK3DInputHandler] \"Hold\" test bind activated.");
      }
      auto tw = inputResultOf(gp.test_while);
      if (tw.while_has_changed) {
         if (tw.active()) {
            qDebug("[DK3DInputHandler] \"While\" test bind has started...");
         } else {
            qDebug("[DK3DInputHandler] \"While\" test bind has ended.");
         }
      }
   }
   DKVulkanCameraUpdate update;
   update.delta_seconds = elapsed;
   update.move.speed    = 1.0;
   {
      auto& km = this->binds.keyboard.camera.move;
      auto& cm = update.move.direction;
      if (inputResultOf(km.forward).active()) {
         cm.y += 1;
      }
      if (inputResultOf(km.back).active()) {
         cm.y -= 1;
      }
      if (inputResultOf(km.left).active()) {
         cm.x -= 1;
      }
      if (inputResultOf(km.right).active()) {
         cm.x += 1;
      }
      if (inputResultOf(km.up).active()) {
         cm.z += 1;
      }
      if (inputResultOf(km.down).active()) {
         cm.z -= 1;
      }
      //
      if (has_gamepad) {
         auto& gm = this->binds.gamepad.camera.move;
         auto  lateral = inputResultOf(gm.lateral);
         if (lateral.active()) {
            cm.x += lateral.x; // stick right is positive; left is negative
            cm.y += lateral.y; // stick up    is positive; down is negative
         }
         if (inputResultOf(gm.up).active()) {
            cm.z += 1;
         }
         if (inputResultOf(gm.down).active()) {
            cm.z -= 1;
         }
      }
   }
   update.turn.speed = glm::radians(90.0F);
   {
      auto& km = this->binds.keyboard.camera.turn;
      auto& cm = update.turn;
      if (inputResultOf(km.left).active()) {
         cm.yaw += 1;
      }
      if (inputResultOf(km.right).active()) {
         cm.yaw -= 1;
      }
      if (inputResultOf(km.up).active()) {
         cm.pitch -= 1;
      }
      if (inputResultOf(km.down).active()) {
         cm.pitch += 1;
      }
      if (has_gamepad) {
         auto& gm    = this->binds.gamepad.camera.turn;
         auto  yaw   = inputResultOf(gm.yaw);
         auto  pitch = inputResultOf(gm.pitch);
         if (yaw.active()) {
            cm.yaw += yaw.x;
         }
         if (pitch.active()) {
            cm.pitch += pitch.x;
         }
      }
      double speed = sqrt((cm.yaw * cm.yaw) + (cm.pitch * cm.pitch));
      speed = std::clamp(speed, 0.0, 1.0);
      update.turn.speed *= speed;
   }

   for (auto& bind : this->binds.keyboard.list) {
      if (!bind.function)
         continue;
      auto r = inputResultOf(bind.input);
      if (r.active() || r.while_has_changed) {
         bind.function->invoke(r, bind.params, update);
      }
   }
   if (has_gamepad) {
      for (auto& bind : this->binds.gamepad.list) {
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
      {  // Gamepad: Test Tap
         auto* t = new DKBoundInputWidget(body);
         t->setObjectName("gamepad.test_tap");
         t->setInputDevice(DKBoundInputWidget::InputDevice::XInput);
         t->setValue(this->binds.gamepad.test_tap);
         layout->addWidget(t);
      }
      {  // Gamepad: Test Hold
         auto* t = new DKBoundInputWidget(body);
         t->setObjectName("gamepad.test_hold");
         t->setInputDevice(DKBoundInputWidget::InputDevice::XInput);
         t->setValue(this->binds.gamepad.test_hold);
         layout->addWidget(t);
      }
      {  // Gamepad: Test While
         auto* t = new DKBoundInputWidget(body);
         t->setObjectName("gamepad.test_while");
         t->setInputDevice(DKBoundInputWidget::InputDevice::XInput);
         t->setValue(this->binds.gamepad.test_while);
         layout->addWidget(t);
      }
      //
      scroll->setWidget(body);
   }
   auto* save = new QPushButton("Save", dialog);
   QObject::connect(save, &QPushButton::clicked, dialog, [this, dialog, body]() {
      this->ignoreAllHeldKeys();
      //
      if (auto* w = body->findChild<DKBoundInputWidget*>("gamepad.test_tap")) {
         this->binds.gamepad.test_tap = w->value();
      }
      if (auto* w = body->findChild<DKBoundInputWidget*>("gamepad.test_hold")) {
         this->binds.gamepad.test_hold = w->value();
      }
      if (auto* w = body->findChild<DKBoundInputWidget*>("gamepad.test_while")) {
         this->binds.gamepad.test_while = w->value();
      }
      //
      dialog->accept();
   });
   layout->addWidget(save, 1, 1);
   layout->addWidget(new DKBoundInputWidget(dialog), 2, 0, 1, 2); // TEST
   //
   dialog->show();
}