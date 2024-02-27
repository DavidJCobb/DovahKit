#pragma once
#include <QString>
#include "./cache_map.h"
#include "./cached_vmad_info.h"

namespace dovahkit::subsystems::form_info_cache {
   struct cache_map_collection {
      cache_map<cached_vmad_info> attached_scripts;
      cache_map<QString>          model_paths;
      cache_map<QString>          quest_filters;

      void clear() {
         attached_scripts.clear();
         model_paths.clear();
         quest_filters.clear();
      }
   };
}