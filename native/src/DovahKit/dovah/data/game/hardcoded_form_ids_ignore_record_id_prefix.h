#pragma once
#include <optional>
#include "../game.h"

namespace dovah::game_feature_support {
   constexpr std::optional<float> hardcoded_form_ids_ignore_record_id_prefix_until_file_version(game g) {
      if (g == game::skyrim_special)
         return 1.71F;
      return {};
   }

   constexpr bool hardcoded_form_ids_always_ignore_record_id_prefix(game g) {
      return !hardcoded_form_ids_ignore_record_id_prefix_until_file_version(g).has_value();
   }

   constexpr bool hardcoded_form_ids_ignore_record_id_prefix(game g, float file_version) {
      auto min_ver_opt = hardcoded_form_ids_ignore_record_id_prefix_until_file_version(g);
      if (min_ver_opt.has_value())
         return file_version < min_ver_opt.value();
      return true;
   }
}