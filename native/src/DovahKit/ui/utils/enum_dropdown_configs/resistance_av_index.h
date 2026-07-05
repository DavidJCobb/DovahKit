#pragma once
#include "./actor_value_index.h"

namespace ui::enum_dropdown_configs {
   inline void resistance_av_index(QComboBox* widget) {
      return actor_value_index<actor_value_index_options{
         .allow_none = true,
         .sorted     = true,
      }>(
         widget,
         [](const dovah::actor_value_info& av_info) {
            if (av_info.type != dovah::actor_value_type::resistance)
               return false;
            return true;
         }
      );
   }
}