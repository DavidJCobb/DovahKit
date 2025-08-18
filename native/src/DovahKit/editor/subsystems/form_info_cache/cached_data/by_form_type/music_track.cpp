#include "./music_track.h"
#include "dovah/files/tes_file_reading/elements.h"
#include "dovah/forms/MusicTrack.h"
#include "dovah/core.h"

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   void music_track::skim_subrecord(dovah::tes_file_reading::subrecord& subrecord) {
      using loaded_form_type = dovah::loaded_forms::MusicTrack;

      switch (subrecord.signature()) {
         case 'CNAM':
            {
               enum class serialized_track_type : uint32_t {
                  palette = 0x23F678C3,
                  single = 0x6ED7E048,
                  silent = 0xA1A9C4D5,
               };
               serialized_track_type v;
               if (subrecord.read(v)) {
                  switch (v) {
                     case serialized_track_type::palette:
                        this->type = music_track_type::palette;
                        break;
                     case serialized_track_type::silent:
                        this->type = music_track_type::silent;
                        break;
                     case serialized_track_type::single:
                        this->type = music_track_type::single;
                        break;
                  }
               }
            }
            break;
      }
   }
   bool music_track::update(const dovah::loaded_forms::MusicTrack& src) {
      using loaded_form_type = dovah::loaded_forms::MusicTrack;

      auto after = src.get_track_type();
      if (after == this->type)
         return false;
      this->type = after;
      return true;
   }
}