#pragma once
#include "dovah/form_types.h"

namespace dovahkit::subsystems::form_info_cache::cacheable_traits {
   struct model_path {
      model_path() = delete;

      static constexpr const auto form_types_of_interest = std::array{
         dovah::form_type::activator,
         dovah::form_type::ammo,
         // TODO: Armors use custom logic where if the top-level folder is "Armor," it's omitted from the tree
         dovah::form_type::body_part_data,
         dovah::form_type::book,
         dovah::form_type::container,
         dovah::form_type::door,
         dovah::form_type::flora,
         dovah::form_type::furniture,
         dovah::form_type::grass,
         dovah::form_type::ingredient,
         dovah::form_type::key,
         dovah::form_type::leveled_character,
         dovah::form_type::light,
         dovah::form_type::misc_item,
         dovah::form_type::movable_static,
         dovah::form_type::potion,
         dovah::form_type::statik,
         dovah::form_type::talking_activator,
         dovah::form_type::tree,
      };

      static constexpr const bool form_type_is_of_interest(dovah::form_type ft) {
         for (auto v : form_types_of_interest)
            if (v == ft)
               return true;
         return false;
      }

      template<typename LoadedFormClass>
      static constexpr const bool form_class_is_of_interest = form_type_is_of_interest(LoadedFormClass::form_type);
   };

}