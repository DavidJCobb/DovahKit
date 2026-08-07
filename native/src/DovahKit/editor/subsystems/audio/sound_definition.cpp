#include "./sound_definition.h"
#include <xaudio2.h>

namespace dovahkit::subsystems::audio {
   /*virtual*/ XAUDIO2_BUFFER_WMA sound_definition::get_xwma_info() const {
      return XAUDIO2_BUFFER_WMA{ nullptr, 0 };
   }
}