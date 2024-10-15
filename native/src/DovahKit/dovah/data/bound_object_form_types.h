#pragma once
#include <array>
#include "../form_types.h"

namespace dovah {
   // All form types that derive from TESBoundObject.
   constexpr const auto bound_object_form_types = std::array{
      form_type::acoustic_space,
      form_type::addon_node,
      form_type::ammo,
      form_type::armor,
      form_type::art_object,
      form_type::book,
      form_type::dual_cast_data,
      form_type::explosion,
      form_type::grass,
      form_type::hazard,
      form_type::idle_marker,
      form_type::leveled_item,
      form_type::leveled_spell,
      form_type::misc_item,
      form_type::note,
      form_type::projectile,
      form_type::statik,
      form_type::static_collection,
      form_type::texture_set,
      form_type::tree,
      form_type::weapon,
      //
      // TESBoundObject -> TESBoundAnimObject -> these:
      //
      form_type::activator,
      form_type::actor_base, // via TESActorBase, not directly from TESNPC
      form_type::container,
      form_type::door,
      form_type::leveled_character,
      form_type::light,
      form_type::sound, // TESSound
      //
      // TESBoundObject -> MagicItem -> these:
      //
      form_type::enchantment,
      form_type::ingredient,
      form_type::potion,
      form_type::spell,
   };
}
