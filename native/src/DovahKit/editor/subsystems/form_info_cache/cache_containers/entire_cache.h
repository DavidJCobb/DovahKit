#pragma once
#include "./data_cache.h"
#include "./presence_cache.h"
#pragma region Data
   #pragma region By form type
      #include "../cached_data/by_form_type/actor_base.h"
      #include "../cached_data/by_form_type/collision_layer.h"
      #include "../cached_data/by_form_type/enchantment.h"
      #include "../cached_data/by_form_type/faction.h"
      #include "../cached_data/by_form_type/head_part.h"
      #include "../cached_data/by_form_type/magic_effect.h"
      #include "../cached_data/by_form_type/music_track.h"
      #include "../cached_data/by_form_type/quest.h"
      #include "../cached_data/by_form_type/package.h"
      #include "../cached_data/by_form_type/topic.h"
      #include "../cached_data/by_form_type/voicetype.h"
   #pragma endregion
   #include "../cached_data/attached_scripts.h"
   #include <QString>
#pragma endregion

namespace dovahkit::subsystems::form_info_cache {
   struct entire_cache {
      struct {
         #ifndef Q_MOC_RUN // see comments re: Qt MOC in the main FIC singleton's header
            #include "./macros/FOR_EACH_CACHED_FORM_TYPE.define.h"
            #define X(_name, ...) data_cache<cached_data::by_form::_name> _name##s; // field name is pluralized
            FOR_EACH_CACHED_FORM_TYPE(X);
            #include "./macros/FOR_EACH_CACHED_FORM_TYPE.undef.h"
         #endif
      } by_form_type;
      data_cache<cached_data::attached_scripts> attached_scripts;
      data_cache<QString> model_paths;
      data_cache<QString> quest_filters;

      presence_cache sharedinfo_topics;
   };
}