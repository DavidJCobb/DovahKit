#pragma once
#define protected public
#include "helpers/type_containers/fixed_map.h"
#include "dovah/form_types.h"
namespace dovahkit::subsystems::form_info_cache {
   namespace cached_data {
      namespace by_form {
         class actor_base;
         class collision_layer;
         class enchantment;
         class faction;
         class head_part;
         class magic_effect;
         class music_track;
         class quest;
         class package;
         class topic;
         class voicetype;
      }
   }
}

namespace dovahkit::subsystems::form_info_cache {
   #pragma push_macro("CASE")
   #undef CASE
   #define CASE(_name) cobb::type_containers::fixed_map_entry<cached_data::by_form::_name, dovah::form_type::_name>

   using cached_data_classes_by_form_type = cobb::type_containers::fixed_map<
      CASE(actor_base),
      CASE(collision_layer),
      CASE(enchantment),
      CASE(faction),
      CASE(head_part),
      CASE(magic_effect),
      CASE(music_track),
      CASE(quest),
      CASE(package),
      CASE(topic),
      CASE(voicetype)//,
   >;

   #undef CASE
   #pragma pop_macro("CASE")
   static_assert(std::is_same_v<typename cached_data_classes_by_form_type::value_type, dovah::form_type>);

   template<dovah::form_type FormType>
   using cached_data_class_by_form_type = cached_data_classes_by_form_type::key_of<FormType>;
}