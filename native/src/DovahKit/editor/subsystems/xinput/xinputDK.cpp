#include "xinputDK.h"
#include <numbers>

namespace dovahkit::subsystems::xinput {
   namespace {
      constexpr bool  joystick_inertia_as_accel    = true;
      constexpr bool  joystick_inertia_subtractive = true; // overridden by inertia-as-accel
      constexpr float joystick_time_to_full_accel  = 0.8;

      struct _dll_version {
         constexpr _dll_version(const TCHAR* f, xinput::dll_version v) : file(f), version(v) {}

         const TCHAR* file;
         xinput::dll_version version;
      };

      constexpr std::array dll_versions = {
         _dll_version(TEXT("XINPUT1_4.DLL"),   dll_version::xinput_1_4),
         _dll_version(TEXT("XINPUT1_3.DLL"),   dll_version::xinput_1_3),
         _dll_version(TEXT("XINPUT9_1_0.DLL"), dll_version::xinput_9_1_0),
      };
   }

   core::core() {
      for (auto& is : this->state.input)
         ZeroMemory(&is, sizeof(is));
      //
      for (auto& dv : dll_versions) {
         this->dll.handle = LoadLibraryEx(dv.file, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
         if (this->dll.handle) {
            this->dll.version = dv.version;
            break;
         }
      }
      if (!this->dll.handle) {
         qDebug("[core] Failed to load the XInput DLL (not supported on this system?).");
         return;
      }
      //
      this->dll.proc.p_XInputGetState.load(this->dll.handle);
      //
      if (this->dll.proc.p_XInputGetState.missing()) {
         qDebug("[core] GetProcAddress failed for XInputGetState; unloading XInput DLL.");
         FreeLibrary(this->dll.handle);
         this->dll.handle = NULL;
         return;
      }
      //
      for (uint8_t i = 0; i < 4; ++i) {
         auto& is     = this->state.input[i];
         auto  result = (this->dll.proc.p_XInputGetState)(i, &is);
         if (result == ERROR_SUCCESS)
            this->state.present |= (1 << i);
      }
      this->state.last_poll          = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      this->state.last_present_check = this->state.last_poll;
   }
   core::~core() {
      if (this->dll.handle) {
         FreeLibrary(this->dll.handle);
         this->dll.handle = NULL;
      }
   }

   QPointF core::normalize_stick(side s, int16_t x, int16_t y) const {
      constexpr float pi      = std::numbers::pi_v<float>;
      constexpr float pi_half = std::numbers::pi_v<double> / 2;
      //
      if (x == -32768)
         ++x;
      if (y == -32768)
         ++y;
      QPointF out = { (qreal)x, (qreal)y };
      out /= 32767;
      //
      float distance_sq = (out.x() * out.x()) + (out.y() * out.y());
      {  // apply inner deadzone
         float threshold = this->stickInnerDeadzone(s);
         if (distance_sq < threshold * threshold)
            return { 0, 0 };
      }
      {  // apply axial deadzone
         float axial = this->stickAxialDeadzone(s);
         if (axial > 0.0) {
            float deg = atan2(out.y(), out.x());
            if (deg < 0.0)
               deg += (pi * 2); // normalize to range [0, 2pi]
            float off = fmod(deg, pi_half); // normalize to range [0, 0.5pi]
            if (off < axial) {
               float distance = sqrt(distance_sq);
               deg = round(deg / pi_half) * pi_half;
               //
               out.rx() = cos(deg) * distance;
               out.ry() = sin(deg) * distance;
            }
         }
      }
      if (this->invertStick(s)) {
         auto& oy = out.ry();
         oy = -oy;
      }
      return out;
   }
   void core::apply_stick_inertia(side s, float elapsed, float& accel, const QPointF& raw, QPointF& out) const {
      constexpr bool use_temporal_inertia = true;
      //
      double factor = std::min(1.0F, this->stickInertiaChaseSpeed(s));
      if (factor <= 0) {
         out = raw;
         return;
      }
      {
         float r_sq = (raw.x() * raw.x()) + (raw.y() * raw.y());
         float o_sq = (out.x() * out.x()) + (out.y() * out.y());
         if (o_sq > r_sq + 0.05) { // user is moving the joystick back to a neutral position
            out = raw;
            if constexpr (joystick_inertia_as_accel)
               accel = std::max(0.0F, accel - elapsed);
            return;
         }
      }
      if constexpr (joystick_inertia_as_accel) {
         accel = std::min(joystick_time_to_full_accel, accel + elapsed);
         QPointF diff = raw - out;
         diff *= (accel / joystick_time_to_full_accel);
         out = out + diff;
      } else {
         factor *= elapsed;
         //
         QPointF diff = raw - out;
         float distance_sq = (diff.rx() * diff.rx()) + (diff.ry() * diff.ry());
         if (distance_sq < factor * factor) {
            out = raw;
            return;
         }
         float scale = sqrt(distance_sq);
         diff /= scale;
         if constexpr (!joystick_inertia_subtractive) {
            diff *= factor;
            out = out + diff; // results in the camera drifting and sliding when you change directions
         } else {
            diff *= (1 - factor);
            out = raw - diff;
         }
      }
   }
   void core::normalize_input_state(float elapsed, gamepad& out, _gamepad_internal_state& internal, const XINPUT_GAMEPAD& state) const {
      out.buttons = state.wButtons;
      out.lt      = float(state.bLeftTrigger)  / 255.0;
      out.rt      = float(state.bRightTrigger) / 255.0;
      out.raw.ls  = normalize_stick(side::left,  state.sThumbLX, state.sThumbLY);
      out.raw.rs  = normalize_stick(side::right, state.sThumbRX, state.sThumbRY);
      this->apply_stick_inertia(side::left,  elapsed, internal.accel.l, out.raw.ls, out.ls);
      this->apply_stick_inertia(side::right, elapsed, internal.accel.r, out.raw.rs, out.rs);
      //out.ls      = normalize_stick(side::left,  state.sThumbLX, state.sThumbLY);
      //out.rs      = normalize_stick(side::right, state.sThumbRX, state.sThumbRY);
   }

   const gamepad& core::gamepadState() const {
      if (!this->state.present)
         return this->state.gamepads[0];
      for (size_t i = 0; i < this->state.input.size(); ++i)
         if (this->state.present & (1 << i))
            return this->state.gamepads[i];
      return this->state.gamepads[0];
   }
   const gamepad& core::gamepadState(size_t i) const {
      if (i >= this->state.gamepads.size())
         throw std::logic_error("out-of-bounds gamepad index");
      return this->state.gamepads[i];
   }
   bool core::isGamepadConnected() const {
      return this->state.present != 0;
   }
   bool core::isGamepadConnected(size_t i) const {
      if (i >= this->state.input.size())
         return false;
      return (this->state.present & (1 << i)) != 0;
   }

   bool core::invertStick(side s) const {
      return s == side::right;
   }
   float core::stickInertiaChaseSpeed(side s) const {
      if (s == side::right)
         return 0.3;
      return 0.0;
   }
   float core::stickAxialDeadzone(side s) const {
      return (15.0 / 180.0) * std::numbers::pi_v<float>; // 15deg
   }
   float core::stickInnerDeadzone(side s) const {
      if (s == side::left)
         return float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 32767;
      return float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 32767;
   }

   void core::update() {
      using elapsed_t = std::chrono::duration<double, std::chrono::seconds::period>;
      //
      if (!this->dll.handle)
         return;
      auto now     = std::chrono::time_point_cast<timestamp_t::duration>(timestamp_t::clock::now());
      auto elapsed = (this->state.last_poll == zero_time) ? (1.0 / 60) : elapsed_t(now - this->state.last_poll).count();
      if (elapsed <= 0.0)
         //
         // This can happen sometimes -- we receive  an update so soon that it's not even 
         // an easily-measurable  fraction of a  second -- and in  that case, there's not 
         // any point to doing input processing. We'll just be scaling speeds and whatnot 
         // by zero anyway.
         //
         return;
      this->state.last_poll = now;
      //
      bool check_only_present = true;
      {
         auto elapsed = (this->state.last_present_check == zero_time) ? (1.0 / 60) : elapsed_t(now - this->state.last_present_check).count();
         if (elapsed >= 3.0) {
            check_only_present = false;
            this->state.last_present_check = now;
         }
      }
      //
      XINPUT_STATE xs;
      for(size_t i = 0; i < this->state.input.size(); ++i) {
         bool present = (this->state.present & (1 << i));
         if (check_only_present) {
            if (!present)
               continue;
         }
         auto result = (this->dll.proc.p_XInputGetState)(i, &xs);
         if (result == ERROR_SUCCESS) {
            this->normalize_input_state(elapsed, this->state.gamepads[i], this->state.gamepad_internal[i], xs.Gamepad);
            this->state.present |= (1 << i);
            if (!present) {
               // TODO: query capabilities
            }
         } else {
            this->state.present &= ~(1 << i);
            this->state.gamepad_internal[i].vibrations.clear();
         }
      }
   }

}