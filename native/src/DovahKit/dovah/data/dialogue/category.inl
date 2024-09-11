#pragma once
#include "./category.h"

namespace dovah::dialogue {
   enum class category : uint8_t {
      topic, // Player Dialogue
      favor_dialogue, // NOT the same as `favors`
      scene,
      combat,
      favors,
      detection,
      service,
      miscellaneous
   };

   constexpr std::string_view internal_name_for_category(category c) {
      switch (c) {
         case category::topic: return "PlayerDialogue";
         case category::favor_dialogue: return "FavorDialogue";
         case category::scene: return "SceneDialogue";
         case category::combat: return "Combat";
         case category::favors: return "Favors";
         case category::detection: return "Detection";
         case category::service: return "Service";
         case category::miscellaneous: return "Miscellaneous";
      }
      return {};
   }
}