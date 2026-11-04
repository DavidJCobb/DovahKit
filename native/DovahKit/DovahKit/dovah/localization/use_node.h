#pragma once
#include <map>
#include <set>
#include <string>
#include "dovah/types/bare_localized_string_id.h"
#include "./category.h"
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
            enum category            category = {};
            bare_localized_string_id id       = 0;
         } used;
         struct {
            // the map values are use counts for each individual user
            std::map<form_stub*,  size_t> forms;
            std::set<std::string>         game_settings; // by name, forced to lowercase
         } users;
   };
}