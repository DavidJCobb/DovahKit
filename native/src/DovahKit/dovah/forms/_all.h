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
   #include "ActorValueInfo.h"
   #include "AddOnNode.h"
   #include "Ammo.h"
   #include "AnimationProp.h"
   #include "Apparatus.h"
   #include "Armor.h"
   #include "ArmorAddon.h"
   #include "ArtObject.h"
   #include "AssociationType.h"
   #include "Book.h"
   #include "CameraPath.h"
   #include "CameraShot.h"
   #include "Cell.h"
   #include "Class.h"
   #include "Climate.h"
   #include "CollisionLayer.h"
   #include "Color.h"
   #include "CombatStyle.h"
   #include "ConstructibleObject.h"
   #include "Container.h"
   #include "Debris.h"
   #include "DefaultObjectManager.h"
   #include "DialogueBranch.h"
   #include "Door.h"
   #include "DualCastData.h"
   #include "EffectShader.h"
   #include "Enchantment.h"
   #include "EncounterZone.h"
   #include "EquipSlot.h"
   #include "Explosion.h"
   #include "Eyes.h"
   #include "Faction.h"
   #include "Flora.h"
   #include "Footstep.h"
   #include "FootstepSet.h"
   #include "FormList.h"
   #include "Furniture.h"
   #include "Global.h"
   #include "Grass.h"
   #include "Hazard.h"
   #include "HeadPart.h"
   #include "IdleAnimation.h"
   #include "IdleMarker.h"
   #include "Imagespace.h"
   #include "ImagespaceModifier.h"
   #include "ImpactData.h"
   #include "ImpactDataSet.h"
   #include "Ingredient.h"
   #include "Key.h"
   #include "Keyword.h"
   #include "Landscape.h"
   #include "LandTexture.h"
   #include "LensFlare.h"
   #include "LeveledCharacter.h"
   #include "LeveledItem.h"
   #include "LeveledSpell.h"
   #include "Light.h"
   #include "LightingTemplate.h"
   #include "LoadingScreen.h"
   #include "Location.h"
   #include "LocationRefType.h"
   #include "MagicEffect.h"
   #include "MaterialObject.h"
   #include "MaterialType.h"
   #include "MenuIcon.h"
   #include "Message.h"
   #include "MiscItem.h"
   #include "MovableStatic.h"
   #include "MovementType.h"
   #include "MusicTrack.h"
   #include "MusicType.h"
   #include "Navmesh.h"
   #include "NavMeshInfoMap.h"
   #include "Note.h"
   #include "ObjectReference.h"
   #include "Outfit.h"
   #include "Package.h"
   #include "Perk.h"
   #include "PlacedArrow.h"
   #include "PlacedBarrier.h"
   #include "PlacedBeam.h"
   #include "PlacedCone.h"
   #include "PlacedFlame.h"
   #include "PlacedGrenade.h"
   #include "PlacedHazard.h"
   #include "PlacedMissile.h"
   #include "Potion.h"
   #include "Projectile.h"
   #include "Quest.h"
   #include "Race.h"
   #include "Ragdoll.h"
   #include "Region.h"
   #include "Relationship.h"
   #include "ReverbParameters.h"
   #include "Scene.h"
   #include "Scroll.h"
   #include "ShaderParticleGeometry.h"
   #include "Shout.h"
   #include "SoulGem.h"
   #include "Sound.h"
   #include "SoundCategory.h"
   #include "SoundDescriptor.h"
   #include "SoundOutputModel.h"
   #include "Spell.h"
   #include "Static.h"
   #include "StaticCollection.h"
   #include "StoryManagerBranchNode.h"
   #include "StoryManagerEventNode.h"
   #include "StoryManagerQuestNode.h"
   #include "TalkingActivator.h"
   #include "TextureSet.h"
   #include "Topic.h"
   #include "TopicInfo.h"
   #include "Tree.h"
   #include "VisualEffect.h"
   #include "Voicetype.h"
   #include "VolumetricLighting.h"
   #include "WaterType.h"
   #include "Weapon.h"
   #include "Weather.h"
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
      loaded_forms::ActorValueInfo,
      loaded_forms::AddOnNode,
      loaded_forms::Ammo,
      loaded_forms::AnimationProp,
      loaded_forms::Apparatus,
      loaded_forms::Armor,
      loaded_forms::ArmorAddon,
      loaded_forms::ArtObject,
      loaded_forms::AssociationType,
      loaded_forms::Book,
      loaded_forms::CameraPath,
      loaded_forms::CameraShot,
      loaded_forms::Cell,
      loaded_forms::Class,
      loaded_forms::Climate,
      loaded_forms::CollisionLayer,
      loaded_forms::Color,
      loaded_forms::CombatStyle,
      loaded_forms::ConstructibleObject,
      loaded_forms::Container,
      loaded_forms::Debris,
      loaded_forms::DefaultObjectManager,
      loaded_forms::DialogueBranch,
      loaded_forms::Door,
      loaded_forms::DualCastData,
      loaded_forms::EffectShader,
      loaded_forms::Enchantment,
      loaded_forms::EncounterZone,
      loaded_forms::EquipSlot,
      loaded_forms::Explosion,
      loaded_forms::Eyes,
      loaded_forms::Faction,
      loaded_forms::Flora,
      loaded_forms::Footstep,
      loaded_forms::FootstepSet,
      loaded_forms::FormList,
      loaded_forms::Furniture,
      loaded_forms::Global,
      loaded_forms::Grass,
      loaded_forms::Hazard,
      loaded_forms::HeadPart,
      loaded_forms::IdleAnimation,
      loaded_forms::IdleMarker,
      loaded_forms::Imagespace,
      loaded_forms::ImagespaceModifier,
      loaded_forms::ImpactData,
      loaded_forms::ImpactDataSet,
      loaded_forms::Ingredient,
      loaded_forms::Key,
      loaded_forms::Keyword,
      loaded_forms::Landscape,
      loaded_forms::LandTexture,
      loaded_forms::LensFlare,
      loaded_forms::LeveledCharacter,
      loaded_forms::LeveledItem,
      loaded_forms::LeveledSpell,
      loaded_forms::Light,
      loaded_forms::LightingTemplate,
      loaded_forms::LoadingScreen,
      loaded_forms::Location,
      loaded_forms::LocationRefType,
      loaded_forms::MagicEffect,
      loaded_forms::MaterialObject,
      loaded_forms::MaterialType,
      loaded_forms::MenuIcon,
      loaded_forms::Message,
      loaded_forms::MiscItem,
      loaded_forms::MovableStatic,
      loaded_forms::MovementType,
      loaded_forms::MusicTrack,
      loaded_forms::MusicType,
      loaded_forms::Navmesh,
      loaded_forms::NavMeshInfoMap,
      loaded_forms::Note,
      loaded_forms::ObjectReference,
      loaded_forms::Outfit,
      loaded_forms::Package,
      loaded_forms::Perk,
      loaded_forms::PlacedArrow,
      loaded_forms::PlacedBarrier,
      loaded_forms::PlacedBeam,
      loaded_forms::PlacedCone,
      loaded_forms::PlacedFlame,
      loaded_forms::PlacedGrenade,
      loaded_forms::PlacedHazard,
      loaded_forms::PlacedMissile,
      loaded_forms::Potion,
      loaded_forms::Projectile,
      loaded_forms::Quest,
      loaded_forms::Race,
      loaded_forms::Ragdoll,
      loaded_forms::Region,
      loaded_forms::Relationship,
      loaded_forms::ReverbParameters,
      loaded_forms::Scene,
      loaded_forms::Scroll,
      loaded_forms::ShaderParticleGeometry,
      loaded_forms::Shout,
      loaded_forms::SoulGem,
      loaded_forms::Sound,
      loaded_forms::SoundCategory,
      loaded_forms::SoundDescriptor,
      loaded_forms::SoundOutputModel,
      loaded_forms::Spell,
      loaded_forms::Static,
      loaded_forms::StaticCollection,
      loaded_forms::StoryManagerBranchNode,
      loaded_forms::StoryManagerEventNode,
      loaded_forms::StoryManagerQuestNode,
      loaded_forms::TalkingActivator,
      loaded_forms::TextureSet,
      loaded_forms::Topic,
      loaded_forms::TopicInfo,
      loaded_forms::Tree,
      loaded_forms::VisualEffect,
      loaded_forms::Voicetype,
      loaded_forms::VolumetricLighting,
      loaded_forms::WaterType,
      loaded_forms::Weapon,
      loaded_forms::Weather,
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
}
