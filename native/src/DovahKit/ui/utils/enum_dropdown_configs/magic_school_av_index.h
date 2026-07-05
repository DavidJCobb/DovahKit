#pragma once
#include "./actor_value_index.h"

namespace ui::enum_dropdown_configs {
   inline void magic_school_av_index(QComboBox* widget) {
      return actor_value_index<actor_value_index_options{
         .allow_none = true,
         .sorted     = true,
      }>(
         widget,
         [](const dovah::actor_value_info& av_info) {
            if (av_info.type != dovah::actor_value_type::skill)
               return false;
            if (!(av_info.flags & dovah::actor_value_info::flag::is_magic_school_skill))
               return false;
            return true;
         }
      );
   }
}