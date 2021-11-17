#include "DKXInputSubsystem.h"
#include <numbers>

namespace {
   struct _dll_version {
      constexpr _dll_version(const TCHAR* f, DKXInputSubsystem::DLLVersion v) : file(f), version(v) {}

      const TCHAR* file;
      DKXInputSubsystem::DLLVersion version;
   };

   constexpr std::array dll_versions = {
      _dll_version(TEXT("XINPUT1_4.DLL"),   DKXInputSubsystem::DLLVersion::XInput_1_4),
      _dll_version(TEXT("XINPUT1_3.DLL"),   DKXInputSubsystem::DLLVersion::XInput_1_3),
      _dll_version(TEXT("XINPUT9_1_0.DLL"), DKXInputSubsystem::DLLVersion::XInput_9_1_0),
   };
}

DKXInputSubsystem::DKXInputSubsystem() {
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
      qDebug("[DKXInputSubsystem] Failed to load the XInput DLL (not supported on this system?).");
      return;
   }
   //
   this->dll.proc.p_XInputGetState.load(this->dll.handle);
   //
   if (this->dll.proc.p_XInputGetState.missing()) {
      qDebug("[DKXInputSubsystem] GetProcAddress failed for XInputGetState; unloading XInput DLL.");
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
DKXInputSubsystem::~DKXInputSubsystem() {
   if (this->dll.handle) {
      FreeLibrary(this->dll.handle);
      this->dll.handle = NULL;
   }
}

QPointF DKXInputSubsystem::normalize_stick(Side s, int16_t x, int16_t y) const {
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
void DKXInputSubsystem::normalize_input_state(Gamepad& out, const XINPUT_GAMEPAD& state) const {
   out.buttons = state.wButtons;
   out.lt      = float(state.bLeftTrigger)  / 32767.0;
   out.rt      = float(state.bRightTrigger) / 32767.0;
   out.ls      = normalize_stick(Side::Left,  state.sThumbLX, state.sThumbLY);
   out.rs      = normalize_stick(Side::Right, state.sThumbRX, state.sThumbRY);
}

const DKXInputSubsystem::Gamepad& DKXInputSubsystem::gamepadState() const {
   if (!this->state.present)
      return this->state.gamepads[0];
   for (size_t i = 0; i < this->state.input.size(); ++i)
      if (this->state.present & (1 << i))
         return this->state.gamepads[i];
   return this->state.gamepads[0];
}
const DKXInputSubsystem::Gamepad& DKXInputSubsystem::gamepadState(size_t i) const {
   if (i >= this->state.gamepads.size())
      throw std::logic_error("out-of-bounds gamepad index");
   return this->state.gamepads[i];
}
bool DKXInputSubsystem::isGamepadConnected() const {
   return this->state.present != 0;
}
bool DKXInputSubsystem::isGamepadConnected(size_t i) const {
   if (i >= this->state.input.size())
      return false;
   return (this->state.present & (1 << i)) != 0;
}

bool DKXInputSubsystem::invertStick(Side s) const {
   return s == Side::Right;
}
float DKXInputSubsystem::stickAxialDeadzone(Side s) const {
   return (15.0 / 180.0) * std::numbers::pi_v<float>; // 15deg
}
float DKXInputSubsystem::stickInnerDeadzone(Side s) const {
   if (s == Side::Left)
      return float(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / 32767;
   return float(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) / 32767;
}

void DKXInputSubsystem::update() {
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
         this->normalize_input_state(this->state.gamepads[i], xs.Gamepad);
         this->state.present |= (1 << i);
         if (!present) {
            // TODO: query capabilities
         }
      } else {
         this->state.present &= ~(1 << i);
         this->state.gamepad_internal[i].vibes.clear();
      }
   }
}