#pragma once
#include <map>
#include <string>
#include "dovah/types/bare_localized_string_id.h"
#include "dovah/types/localized_string_type.h"
namespace dovah {
   namespace tes {
      class file;
   }
   class form_stub;
   class loaded_game_setting;
}

namespace dovah::localization {
   struct use_node {
      public:
         struct {
            const tes::file*         tes_file = nullptr;
            localized_string_type    type     = {};
            bare_localized_string_id id       = 0;
         } used;
         struct {
            // the map values are use counts for each individual user
            std::map<form_stub*,  size_t> forms;
            std::map<std::string, size_t> game_settings; // by name, forced to lowercase
         } users;
   };
}