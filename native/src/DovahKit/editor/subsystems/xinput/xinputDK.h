#pragma once
#include <array>
#include <cstdint>
#include <QObject>
#include <QVector>
#include <windows.h> // required or the XInput header will make MSVC choke
#include <xinput.h>
#include "helpers/intrusive_windows_defines.h"
#include "./enums/button.h"
#include "./enums/dll_version.h"
#include "./enums/side.h"
#include "./chrono.h"
#include "./gamepad.h"
#include "./vibration.h"

namespace dovahkit::subsystems::xinput {
   class core : public QObject {
      Q_OBJECT;
      public:
         static core& get() {
            static core instance;
            return instance;
         }

      protected:
         struct _gamepad_internal_state {
            QVector<vibration> vibrations;
            struct {
               float l = 0.0;
               float r = 0.0;
            } accel;
         };

      protected:
         core();
         ~core();

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
            HMODULE     handle  = NULL;
            dll_version version = dll_version::none;
            struct {
               #pragma push_macro("xinput_proc")
               #define xinput_proc(name) proc<&name, #name> p_##name;
               xinput_proc(XInputGetState);
               #undef  xinput_proc
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
            std::array<gamepad, 4> gamepads; // "processed" state, e.g. input with deadzones applied
            std::array<_gamepad_internal_state, 4> gamepad_internal;
         } state;

         QPointF normalize_stick(side, int16_t x, int16_t y) const;
         void apply_stick_inertia(side, float elapsed, float& accel, const QPointF& raw, QPointF& out) const;
         void normalize_input_state(float elapsed, gamepad&, _gamepad_internal_state&, const XINPUT_GAMEPAD&) const;

      public:
         inline bool isXInputLoaded() const noexcept { return this->dll.handle != NULL; }

         const gamepad& gamepadState() const; // returns the first connected gamepad, falling back to gamepad 0 if none are connected
         const gamepad& gamepadState(size_t i) const;
         bool isGamepadConnected() const;
         bool isGamepadConnected(size_t i) const;

         bool invertStick(side) const;
         float stickInertiaChaseSpeed(side) const; // per second; in the range (0, 1]; higher = faster; zero or negative is treated as "no inertia"
         float stickAxialDeadzone(side) const; // radians; negative values should be treated as zero
         float stickInnerDeadzone(side) const; // in the range [0, 1]

      public slots:
         void update();
   };
}