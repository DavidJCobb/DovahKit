#pragma once
#include <optional>
#include "./_base.h"
#include "../_base_macros.define.h"
#include "dovah/data/music_track_type.h"
#include "dovah/data/sex.h"

namespace dovah::loaded_forms {
   class MusicTrack;
}

namespace dovahkit::subsystems::form_info_cache::cached_data::by_form {
   class music_track : public _base {
      public:
         static constexpr const auto form_types_of_interest = std::array{ dovah::form_type::music_track };
         static constexpr const auto subrecords_of_interest = std::array{ 'CNAM' };
         MAKE_FORM_INFO_CACHE_DATA_TEMPLATES;

      public:
         using music_track_type = dovah::music_track_type;

      public:
         music_track_type type = music_track_type::palette;

      public:
         constexpr music_track() {}
         void skim_subrecord(dovah::tes_file_reading::subrecord&);

         // Returns true if anything has changed.
         bool update(const dovah::loaded_forms::MusicTrack&);

         constexpr bool operator==(const music_track&) const noexcept = default;
   };
}


#include "../_base_macros.undef.h"