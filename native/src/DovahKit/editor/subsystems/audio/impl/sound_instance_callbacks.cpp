#include "./sound_instance_callbacks.h"
#include "../sound_instance.h"

namespace dovahkit::subsystems::audio::impl {
   void sound_instance_callbacks::OnStreamEnd() {
      this->owner.on_playback_finished({});
   }
}