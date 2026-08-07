#include "./sound_definition.h"
#include <xaudio2.h>
#include "./impl/wave_format_ex.h"

namespace dovahkit::subsystems::audio {
   /*virtual*/ XAUDIO2_BUFFER_WMA sound_definition::get_xwma_info() const {
      return XAUDIO2_BUFFER_WMA{ nullptr, 0 };
   }
   /*virtual*/ size_t sound_definition::estimated_samples_at_time_point(double v) const {
      const auto& format = this->get_format();
      return format.nSamplesPerSec * v;
   }
}