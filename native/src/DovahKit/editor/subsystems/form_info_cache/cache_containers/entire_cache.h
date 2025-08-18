#pragma once
#include "./data_cache.h"
#include "./presence_cache.h"
#pragma region Data
   #pragma region By form type
      #include "../cached_data/by_form_type/actor_base.h"
      #include "../cached_data/by_form_type/faction.h"
      #include "../cached_data/by_form_type/head_part.h"
      #include "../cached_data/by_form_type/magic_effect.h"
      #include "../cached_data/by_form_type/music_track.h"
      #include "../cached_data/by_form_type/package.h"
      #include "../cached_data/by_form_type/voicetype.h"
   #pragma endregion
   #include "../cached_data/attached_scripts.h"
   #include <QString>
#pragma endregion

namespace dovahkit::subsystems::form_info_cache {
   struct entire_cache {
      struct {
         data_cache<cached_data::by_form::actor_base>   actor_bases;
         data_cache<cached_data::by_form::faction>      factions;
         data_cache<cached_data::by_form::head_part>    head_parts;
         data_cache<cached_data::by_form::magic_effect> magic_effects;
         data_cache<cached_data::by_form::music_track>  music_tracks;
         data_cache<cached_data::by_form::package>      packages;
         data_cache<cached_data::by_form::voicetype>    voicetypes;
      } by_form_type;
      data_cache<cached_data::attached_scripts> attached_scripts;
      data_cache<QString> model_paths;
      data_cache<QString> quest_filters;

      presence_cache sharedinfo_topics;
   };
}