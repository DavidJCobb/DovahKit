#pragma once
#include <iterator> // std::distance
#include <string>
#include "./topic_subtype.h"

namespace dovah::dialogue {
   constexpr std::string topic_subtype::game_setting_for_name() const {
      std::string out = "sTopicSubtypeText";
      out += internal_name_for_category(this->category);
      out += this->internal_name;
      return out;
   }

   constexpr const topic_subtype* topic_subtype_by_signature(uint32_t signature) {
      for (const topic_subtype& item : all_topic_subtypes)
         if (item.signature == signature)
            return &item;
      return nullptr;
   }
   constexpr size_t topic_subtype_signature_to_index(uint32_t signature) {
      for (size_t i = 0; i < all_topic_subtypes.size(); ++i)
         if (all_topic_subtypes[i].signature == signature)
            return i;
      return (size_t)-1;
   }

   constexpr size_t topic_subtype_index(const topic_subtype& item) {
      return std::distance(all_topic_subtypes.data(), &item);
   }

   constexpr const topic_subtype* default_subtype_for_category(category c) {
      switch (c) {
         case category::topic:
            return topic_subtype_by_signature('CUST');
         case category::scene:
            return topic_subtype_by_signature('SCEN');
         case category::favor_dialogue:
            return topic_subtype_by_signature('FVDL');
      }
      return nullptr;
   }
}
