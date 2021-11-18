#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <QObject>
#include <windows.h>
#include <xinput.h>
#include "../../helpers/intrusive_windows_defines.h"

class DKXInputSubsystem : public QObject {
   Q_OBJECT;
   protected:
      using timestamp_t = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
      static constexpr auto zero_time = timestamp_t(timestamp_t::duration::zero());

   public:
      static DKXInputSubsystem& get() {
         static DKXInputSubsystem instance;
         return instance;
      }

      enum class Button {
         None = 0,
         A           = XINPUT_GAMEPAD_A,
         B           = XINPUT_GAMEPAD_B,
         X           = XINPUT_GAMEPAD_X,
         Y           = XINPUT_GAMEPAD_Y,
         Start       = XINPUT_GAMEPAD_START,
         Back        = XINPUT_GAMEPAD_BACK,
         DPadUp      = XINPUT_GAMEPAD_DPAD_UP,
         DPadDown    = XINPUT_GAMEPAD_DPAD_DOWN,
         DPadLeft    = XINPUT_GAMEPAD_DPAD_LEFT,
         DPadRight   = XINPUT_GAMEPAD_DPAD_RIGHT,
         LS          = XINPUT_GAMEPAD_LEFT_THUMB,
         LeftStick   = XINPUT_GAMEPAD_LEFT_THUMB,
         RS          = XINPUT_GAMEPAD_RIGHT_THUMB,
         RightStick  = XINPUT_GAMEPAD_RIGHT_THUMB,
         LB          = XINPUT_GAMEPAD_LEFT_SHOULDER,
         LeftBumper  = XINPUT_GAMEPAD_LEFT_SHOULDER,
         RB          = XINPUT_GAMEPAD_RIGHT_SHOULDER,
         RightBumper = XINPUT_GAMEPAD_RIGHT_SHOULDER,
      };
      enum class Side {
         Left,
         Right,
      };

      enum class DLLVersion {
         None,
         XInput_1_4,
         XInput_1_3,
         XInput_9_1_0,
      };

      struct Gamepad {
         struct {
            QPointF ls = {};
            QPointF rs = {};
         } raw;
         QPointF  ls = {};
         QPointF  rs = {};
         float    lt = 0;
         float    rt = 0;
         uint16_t buttons = 0;
         //
         inline bool isButtonDown(Button b) const noexcept { return (this->buttons & (uint16_t)b) != 0; }
      };

      using VibrationFunc = float(*)(float percent, float magnitude);
      static float defaultVibrationFunction(float p, float m) {
         return m * p;
      }

   protected:
      struct Vibration {
         timestamp_t   start;
         float         duration  = 0.0;
         float         magnitude = 0.0;
         VibrationFunc easing    = &defaultVibrationFunction;
      };
      struct GamepadInternal {
         QVector<Vibration> vibes;
      };

   protected:
      DKXInputSubsystem();
      ~DKXInputSubsystem();

      template<size_t N> struct _proc_name {
         constexpr _proc_name(const char(&str)[N]) {
            std::copy_n(str, N, value);
         }
         char value[N];
      };
      template<auto F, _proc_name N> struct proc {
         decltype(F) pointer = nullptr;
         //
         inline constexpr const char* name() const noexcept { return N.value; }
         inline bool missing() const noexcept { return this->pointer == nullptr; }
         template<typename ...T> auto operator()(T... args) { return (this->pointer)(args...); }
         //
         inline void load(HMODULE m) {
            this->pointer = (decltype(F)) GetProcAddress(m, this->name());
         }
      };

      struct {
         HMODULE    handle  = NULL;
         DLLVersion version = DLLVersion::None;
         struct {
            #pragma push_macro("xinput_proc")
            #define xinput_proc(name) proc<&name, #name> p_##name;
            xinput_proc(XInputGetState);
            #pragma pop_macro("xinput_proc")
         } proc;
      } dll;
      struct {
         timestamp_t last_poll          = zero_time;
         timestamp_t last_present_check = zero_time;
         //
         uint8_t present = 0;
         std::array<XINPUT_STATE, 4> input;
         //
         std::array<Gamepad, 4> gamepads; // "processed" state, e.g. input with deadzones applied
         std::array<GamepadInternal, 4> gamepad_internal;
      } state;

      QPointF normalize_stick(Side, int16_t x, int16_t y) const;
      void apply_stick_inertia(Side, float elapsed, const QPointF& raw, QPointF& out) const;
      void normalize_input_state(float elapsed, Gamepad&, const XINPUT_GAMEPAD&) const;

   public:
      inline bool isXInputLoaded() const noexcept { return this->dll.handle != NULL; }

      const Gamepad& gamepadState() const; // returns the first connected gamepad, falling back to gamepad 0 if none are connected
      const Gamepad& gamepadState(size_t i) const;
      bool isGamepadConnected() const;
      bool isGamepadConnected(size_t i) const;

      bool invertStick(Side) const;
      float stickInertiaChaseSpeed(Side) const; // per second; in the range (0, 1]; higher = faster; zero or negative is treated as "no inertia"
      float stickAxialDeadzone(Side) const; // radians; negative values should be treated as zero
      float stickInnerDeadzone(Side) const; // in the range [0, 1]

   public slots:
      void update();
};