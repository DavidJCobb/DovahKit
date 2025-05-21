#pragma once
#include <concepts>
#include "helpers/class_array.h"
#include "../../incomplete_code_warnings.h"
//
#include "Form.h"
#pragma region Forms
   #include "AcousticSpace.h"
   #include "Activator.h"
   #include "Actor.h"
   #include "ActorAction.h"
   #include "ActorBase.h"
   #include "AddOnNode.h"
   #include "Ammo.h"
   #include "AnimationProp.h"
   #include "ArtObject.h"
   #include "AssociationType.h"
   #include "CameraShot.h"
   #include "Cell.h"
   #include "Class.h"
   #include "CollisionLayer.h"
   #include "Color.h"
   #include "Container.h"
   #include "DefaultObjectManager.h"
   #include "DialogueBranch.h"
   #include "Door.h"
   #include "DualCastData.h"
   #include "EquipSlot.h"
   #include "Explosion.h"
   #include "Faction.h"
   #include "Flora.h"
   #include "Footstep.h"
   #include "FormList.h"
   #include "Global.h"
   #include "Grass.h"
   #include "Hazard.h"
   #include "HeadPart.h"
   #include "ImpactData.h"
   #include "Key.h"
   #include "Keyword.h"
   #include "Landscape.h"
   #include "LandTexture.h"
   #include "LeveledCharacter.h"
   #include "LeveledItem.h"
   #include "LeveledSpell.h"
   #include "Light.h"
   #include "Location.h"
   #include "LocationRefType.h"
   #include "MagicEffect.h"
   #include "MiscItem.h"
   #include "MovementType.h"
   #include "MusicType.h"
   #include "Note.h"
   #include "ObjectReference.h"
   #include "Outfit.h"
   #include "Package.h"
   #include "Quest.h"
   #include "Race.h"
   #include "Relationship.h"
   #include "ReverbParameters.h"
   #include "Scene.h"
   #include "Shout.h"
   #include "SoulGem.h"
   #include "Sound.h"
   #include "SoundCategory.h"
   #include "SoundDescriptor.h"
   #include "SoundOutputModel.h"
   #include "Static.h"
   #include "TextureSet.h"
   #include "Topic.h"
   #include "TopicInfo.h"
   #include "VisualEffect.h"
   #include "Voicetype.h"
   #include "Weapon.h"
   #include "WordOfPower.h"
   #include "Worldspace.h"
#pragma endregion

namespace dovah {
   using all_loaded_form_types = cobb::class_array<
      loaded_forms::Form,
      //
      loaded_forms::AcousticSpace,
      loaded_forms::Activator,
      loaded_forms::Actor,
      loaded_forms::ActorAction,
      loaded_forms::ActorBase,
      loaded_forms::AddOnNode,
      loaded_forms::Ammo,
      loaded_forms::AnimationProp,
      loaded_forms::ArtObject,
      loaded_forms::AssociationType,
      loaded_forms::CameraShot,
      loaded_forms::Cell,
      loaded_forms::Class,
      loaded_forms::CollisionLayer,
      loaded_forms::Color,
      loaded_forms::Container,
      loaded_forms::DefaultObjectManager,
      loaded_forms::DialogueBranch,
      loaded_forms::Door,
      loaded_forms::DualCastData,
      loaded_forms::EquipSlot,
      loaded_forms::Explosion,
      loaded_forms::Faction,
      loaded_forms::Flora,
      loaded_forms::Footstep,
      loaded_forms::FormList,
      loaded_forms::Global,
      loaded_forms::Grass,
      loaded_forms::Hazard,
      loaded_forms::HeadPart,
      loaded_forms::ImpactData,
      loaded_forms::Key,
      loaded_forms::Keyword,
      loaded_forms::Landscape,
      loaded_forms::LandTexture,
      loaded_forms::Light,
      loaded_forms::LeveledCharacter,
      loaded_forms::LeveledItem,
      loaded_forms::LeveledSpell,
      loaded_forms::Location,
      loaded_forms::LocationRefType,
      loaded_forms::MiscItem,
      loaded_forms::MovementType,
      loaded_forms::MusicType,
      loaded_forms::Note,
      loaded_forms::ObjectReference,
      loaded_forms::Outfit,
      loaded_forms::Package,
      loaded_forms::Quest,
      loaded_forms::Race,
      loaded_forms::Relationship,
      loaded_forms::ReverbParameters,
      loaded_forms::Scene,
      loaded_forms::Shout,
      loaded_forms::SoulGem,
      loaded_forms::Sound,
      loaded_forms::SoundCategory,
      loaded_forms::SoundDescriptor,
      loaded_forms::SoundOutputModel,
      loaded_forms::Static,
      loaded_forms::TextureSet,
      loaded_forms::Topic,
      loaded_forms::TopicInfo,
      loaded_forms::VisualEffect,
      loaded_forms::Voicetype,
      loaded_forms::Weapon,
      loaded_forms::WordOfPower,
      loaded_forms::Worldspace//,
   >;

   namespace impl {
      template<typename T> concept valid_loaded_form_class = requires {
         requires (std::is_same_v<T, loaded_forms::Form> || std::is_base_of_v<loaded_forms::Form, T>);
         { T::form_type } -> std::common_with<form_type>;
      };
      constexpr size_t first_invalid_form_class = all_loaded_form_types::index_of_matching_type<[]<typename T>() { return !valid_loaded_form_class<T>; }>;
   }
   static_assert(
      impl::first_invalid_form_class == (size_t)-1,
      "All loaded-form classes must have a static `form_type` member of type `enum form_type`."
   );

   static_assert(
      incomplete_code_warnings::allow_compiling_despite_incomplete_forms || !all_loaded_form_types::contains_matching_type<[]<typename T>() { return std::is_base_of_v<loaded_forms::_IncompleteFormType, T>; }>,
      "The backend for one or more form types is incomplete."
   );
}
