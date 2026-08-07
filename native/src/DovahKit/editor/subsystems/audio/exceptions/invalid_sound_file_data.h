#pragma once
#include <stdexcept>

namespace dovahkit::subsystems::audio::exceptions {
   class invalid_sound_file_data : public std::exception {
      public:
         invalid_sound_file_data() : exception("the provided sound data isn't of the requested type") {}
   };
}