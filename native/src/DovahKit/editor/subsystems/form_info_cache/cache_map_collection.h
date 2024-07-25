#pragma once
#include <QString>
#include "./cache_map.h"
#include "./cached_faction_info.h"
#include "./cached_head_part_info.h"
#include "./cached_vmad_info.h"
#include "./cached_voicetype_info.h"

namespace dovahkit::subsystems::form_info_cache {
   struct cache_map_collection {
      cache_map<cached_vmad_info>      attached_scripts;
      cache_map<cached_faction_info>   factions;
      cache_map<cached_head_part_info> head_parts;
      cache_map<QString>               model_paths;
      cache_map<QString>               quest_filters;
      cache_map<cached_voicetype_info> voicetypes;

      void clear() {
         attached_scripts.clear();
         factions.clear();
         head_parts.clear();
         model_paths.clear();
         quest_filters.clear();
         voicetypes.clear();
      }
   };
}