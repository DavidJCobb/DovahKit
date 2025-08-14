#include "open_window_for_form.h"
#include <QMessageBox>
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_dialogue_branch_quest.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "./core.h"
#include "../ui/main_window/form_use_info.h"
#include "../ui/form_windows/acoustic_space.h"
#include "../ui/form_windows/activator.h"
#include "../ui/form_windows/actor_action.h"
#include "../ui/form_windows/actor_base.h"
#include "../ui/form_windows/addon_node.h"
#include "../ui/form_windows/ammo.h"
#include "../ui/form_windows/animation_prop.h"
#include "../ui/form_windows/art_object.h"
#include "../ui/form_windows/association_type.h"
#include "../ui/form_windows/book.h"
#include "../ui/form_windows/camera_shot.h"
#include "../ui/form_windows/cell.h"
#include "../ui/form_windows/class.h"
#include "../ui/form_windows/climate.h"
#include "../ui/form_windows/collision_layer.h"
#include "../ui/form_windows/color.h"
#include "../ui/form_windows/combat_style.h"
#include "../ui/form_windows/container.h"
#include "../ui/form_windows/dialogue_branch.h"
#include "../ui/form_windows/door.h"
#include "../ui/form_windows/dual_cast_data.h"
#include "../ui/form_windows/effectshader.h"
#include "../ui/form_windows/enchantment.h"
#include "../ui/form_windows/encounter_zone.h"
#include "../ui/form_windows/equip_slot.h"
#include "../ui/form_windows/explosion.h"
#include "../ui/form_windows/faction.h"
#include "../ui/form_windows/flora.h"
#include "../ui/form_windows/footstep.h"
#include "../ui/form_windows/formlist.h"
#include "../ui/form_windows/global.h"
#include "../ui/form_windows/grass.h"
#include "../ui/form_windows/hazard.h"
#include "../ui/form_windows/head_part.h"
#include "../ui/form_windows/impact_data.h"
#include "../ui/form_windows/ingredient.h"
#include "../ui/form_windows/key.h"
#include "../ui/form_windows/keyword.h"
#include "../ui/form_windows/landtexture.h"
#include "../ui/form_windows/leveled_character.h"
#include "../ui/form_windows/leveled_item.h"
#include "../ui/form_windows/leveled_spell.h"
#include "../ui/form_windows/light.h"
#include "../ui/form_windows/loading_screen.h"
#include "../ui/form_windows/location_ref_type.h"
#include "../ui/form_windows/magic_effect.h"
#include "../ui/form_windows/material_object.h"
#include "../ui/form_windows/material_type.h"
#include "../ui/form_windows/misc_item.h"
#include "../ui/form_windows/movement_type.h"
#include "../ui/form_windows/music_type.h"
#include "../ui/form_windows/note.h"
#include "../ui/form_windows/outfit.h"
#include "../ui/form_windows/potion.h"
#include "../ui/form_windows/quest.h"
#include "../ui/form_windows/race.h"
#include "../ui/form_windows/relationship.h"
#include "../ui/form_windows/reverb_parameters.h"
#include "../ui/form_windows/scroll.h"
#include "../ui/form_windows/shout.h"
#include "../ui/form_windows/soul_gem.h"
#include "../ui/form_windows/sound.h"
#include "../ui/form_windows/sound_category.h"
#include "../ui/form_windows/sound_descriptor.h"
#include "../ui/form_windows/sound_output_model.h"
#include "../ui/form_windows/spell.h"
#include "../ui/form_windows/static.h"
#include "../ui/form_windows/talking_activator.h"
#include "../ui/form_windows/textureset.h"
#include "../ui/form_windows/topic.h"
#include "../ui/form_windows/topic_info.h"
#include "../ui/form_windows/visual_effect.h"
#include "../ui/form_windows/voicetype.h"
#include "../ui/form_windows/weapon.h"
#include "../ui/form_windows/word_of_power.h"
#include "ui/main_window.h" // MainWindow::get

namespace {
   template<typename T> QDialog* _make(dovah::form_stub& f, QWidget* p) {
      return new T(f, p);
   }

   constexpr const auto factory = std::array{
      std::pair{ dovah::form_type::acoustic_space,    _make<FormDialogAcousticSpace> },
      std::pair{ dovah::form_type::activator,         _make<FormDialogActivator> },
      std::pair{ dovah::form_type::action,            _make<FormDialogActorAction> },
      std::pair{ dovah::form_type::actor_base,        _make<FormDialogActorBase> },
      std::pair{ dovah::form_type::addon_node,        _make<FormDialogAddOnNode> },
      std::pair{ dovah::form_type::ammo,              _make<FormDialogAmmo> },
      std::pair{ dovah::form_type::animation_prop,    _make<FormDialogAnimationProp> },
      std::pair{ dovah::form_type::art_object,        _make<FormDialogArtObject> },
      std::pair{ dovah::form_type::association_type,  _make<FormDialogAssociationType> },
      std::pair{ dovah::form_type::book,              _make<FormDialogBook> },
      std::pair{ dovah::form_type::camera_shot,       _make<FormDialogCameraShot> },
      std::pair{ dovah::form_type::cell,              _make<FormDialogCell> },
      std::pair{ dovah::form_type::climate,           _make<FormDialogClimate> },
      std::pair{ dovah::form_type::combat_class,      _make<FormDialogClass> },
      std::pair{ dovah::form_type::collision_layer,   _make<FormDialogCollisionLayer> },
      std::pair{ dovah::form_type::color,             _make<FormDialogColor> },
      std::pair{ dovah::form_type::combat_style,      _make<FormDialogCombatStyle> },
      std::pair{ dovah::form_type::container,         _make<FormDialogContainer> },
      std::pair{ dovah::form_type::dialogue_branch,   _make<FormDialogDialogueBranch> },
      std::pair{ dovah::form_type::door,              _make<FormDialogDoor> },
      std::pair{ dovah::form_type::dual_cast_data,    _make<FormDialogDualCastData> },
      std::pair{ dovah::form_type::effect_shader,     _make<FormDialogEffectShader> },
      std::pair{ dovah::form_type::enchantment,       _make<FormDialogEnchantment> },
      std::pair{ dovah::form_type::encounter_zone,    _make<FormDialogEncounterZone> },
      std::pair{ dovah::form_type::equip_slot,        _make<FormDialogEquipSlot> },
      std::pair{ dovah::form_type::explosion,         _make<FormDialogExplosion> },
      std::pair{ dovah::form_type::faction,           _make<FormDialogFaction> },
      std::pair{ dovah::form_type::flora,             _make<FormDialogFlora> },
      std::pair{ dovah::form_type::footstep,          _make<FormDialogFootstep> },
      std::pair{ dovah::form_type::formlist,          _make<FormDialogFormList> },
      std::pair{ dovah::form_type::global,            _make<FormDialogGlobal> },
      std::pair{ dovah::form_type::grass,             _make<FormDialogGrass> },
      std::pair{ dovah::form_type::hazard,            _make<FormDialogHazard> },
      std::pair{ dovah::form_type::head_part,         _make<FormDialogHeadPart> },
      std::pair{ dovah::form_type::impact_data,       _make<FormDialogImpactData> },
      std::pair{ dovah::form_type::ingredient,        _make<FormDialogIngredient> },
      std::pair{ dovah::form_type::key,               _make<FormDialogKey> },
      std::pair{ dovah::form_type::keyword,           _make<FormDialogKeyword> },
      std::pair{ dovah::form_type::land_texture,      _make<FormDialogLandTexture> },
      std::pair{ dovah::form_type::leveled_character, _make<FormDialogLeveledCharacter> },
      std::pair{ dovah::form_type::leveled_item,      _make<FormDialogLeveledItem> },
      std::pair{ dovah::form_type::leveled_spell,     _make<FormDialogLeveledSpell> },
      std::pair{ dovah::form_type::light,             _make<FormDialogLight> },
      std::pair{ dovah::form_type::loading_screen,    _make<FormDialogLoadingScreen> },
      std::pair{ dovah::form_type::location_ref_type, _make<FormDialogLocationRefType> },
      std::pair{ dovah::form_type::magic_effect,      _make<FormDialogMagicEffect> },
      std::pair{ dovah::form_type::material_object,   _make<FormDialogMaterialObject> },
      std::pair{ dovah::form_type::material_type,     _make<FormDialogMaterialType> },
      std::pair{ dovah::form_type::misc_item,         _make<FormDialogMiscItem> },
      std::pair{ dovah::form_type::movement_type,     _make<FormDialogMovementType> },
      std::pair{ dovah::form_type::music_type,        _make<FormDialogMusicType> },
      std::pair{ dovah::form_type::note,              _make<FormDialogNote> },
      std::pair{ dovah::form_type::outfit,            _make<FormDialogOutfit> },
      std::pair{ dovah::form_type::potion,            _make<FormDialogPotion> },
      std::pair{ dovah::form_type::quest,             _make<FormDialogQuest> },
      std::pair{ dovah::form_type::race,              _make<FormDialogRace> },
      std::pair{ dovah::form_type::relationship,      _make<FormDialogRelationship> },
      std::pair{ dovah::form_type::reverb_parameters, _make<FormDialogReverbParameters> },
      std::pair{ dovah::form_type::scroll,            _make<FormDialogScroll> },
      std::pair{ dovah::form_type::shout,             _make<FormDialogShout> },
      std::pair{ dovah::form_type::soul_gem,          _make<FormDialogSoulGem> },
      std::pair{ dovah::form_type::sound,             _make<FormDialogSound> },
      std::pair{ dovah::form_type::sound_category,    _make<FormDialogSoundCategory> },
      std::pair{ dovah::form_type::sound_descriptor,  _make<FormDialogSoundDescriptor> },
      std::pair{ dovah::form_type::sound_output_model, _make<FormDialogSoundOutputModel> },
      std::pair{ dovah::form_type::spell,             _make<FormDialogSpell> },
      std::pair{ dovah::form_type::statik,            _make<FormDialogStatic> },
      std::pair{ dovah::form_type::talking_activator, _make<FormDialogTalkingActivator> },
      std::pair{ dovah::form_type::texture_set,       _make<FormDialogTextureSet> },
      std::pair{ dovah::form_type::topic,             _make<FormDialogTopic> },
      std::pair{ dovah::form_type::topic_info,        _make<FormDialogTopicInfo> },
      std::pair{ dovah::form_type::visual_effect,     _make<FormDialogVisualEffect> },
      std::pair{ dovah::form_type::voicetype,         _make<FormDialogVoicetype> },
      std::pair{ dovah::form_type::weapon,            _make<FormDialogWeapon> },
      std::pair{ dovah::form_type::word_of_power,     _make<FormDialogWordOfPower> },
   };

   #pragma region Compile-time sanity checks
   static_assert(
      []() -> bool {
         constexpr auto size = factory.size();
         for (size_t i = 0; i < size; ++i) {
            auto ft = factory[i].first;
            for (size_t j = 0; j < i; ++j) {
               if (factory[j].first == ft)
                  return false;
            }
         }
         return true;
      }(),
      "The factory is misconfigured: a form type is specified multiple times."
   );
   static_assert(
      []() -> bool {
         constexpr auto size = factory.size();
         for (size_t i = 0; i < size; ++i) {
            bool is_ref = dovah::form_type_is_reference(factory[i].first); // make an exception for refs, because REFR subclasses will generally share the same UI as REFR
            auto func   = factory[i].second;
            for (size_t j = 0; j < i; ++j) {
               if (factory[j].second == func) {
                  bool also_ref = dovah::form_type_is_reference(factory[j].first);
                  if (!(is_ref && also_ref))
                     return false;
               }
            }
         }
         return true;
      }(),
      "The factory is misconfigured: multiple form types share the same dialog."
   );
   #pragma endregion
}

void open_use_info_dialog_for_form(dovah::form_stub& stub, QWidget* parent) {
   //
   // First, let's check if there's already a window for this form. If so, we should just 
   // refocus that window instead of opening a new one.
   //
   auto  formID = stub.formID;
   auto& editor = DovahKitCore::get();
   auto  it     = editor.extant_use_info_dialogs.find(formID);
   if (it != editor.extant_use_info_dialogs.end()) {
      auto dialog = it->second;
      if (dialog) {
         dialog->raise();
         dialog->activateWindow();
         return;
      }
   }
   //
   // If we made it to here, then there isn't already a window for this form, so let's 
   // open one.
   //
   auto dialog = new FormUseInfoDialog(&stub, parent);
   editor.extant_use_info_dialogs[stub.formID] = dialog;
   QObject::connect(dialog, &QDialog::finished, &editor, [formID, dialog]() {
      auto& editor = DovahKitCore::get();
      auto& map    = editor.extant_use_info_dialogs;
      auto  it     = map.find(formID);
      if (it != map.end())
         map.erase(it);
      //
      dialog->deleteLater();
   });
   dialog->show();
}
void open_edit_dialog_for_form(dovah::form_stub& stub, QWidget* parent) {
   auto& editor = DovahKitCore::get();

   auto _get_extant_dialog = [](dovah::form_stub& stub) -> QDialog* {
      auto& editor = DovahKitCore::get();
      auto  it     = editor.extant_form_edit_dialogs.find(stub.formID);
      if (it != editor.extant_form_edit_dialogs.end()) {
         return it->second;
      }
      return nullptr;
   };

   //
   // First, let's check if there's already a window for this form. If so, we should just 
   // refocus that window instead of opening a new one.
   //
   if (auto* dialog = _get_extant_dialog(stub)) {
      dialog->raise();
      dialog->activateWindow();
      return;
   }
   //
   // If we made it to here, then there isn't already a window for this form, so let's 
   // open one.
   //
   if (parent == nullptr) {
      switch (stub.form_type) {
         //
         // Dialogue-related forms should be accessed via the edit dialog for their 
         // associated quest.
         //
         case dovah::form_type::dialogue_branch:
         case dovah::form_type::topic:
         case dovah::form_type::topic_info:
            {
               dovah::form_stub* quest_stub = nullptr;
               switch (stub.form_type) {
                  case dovah::form_type::dialogue_branch:
                     quest_stub = dovah::form_stub_helpers::get_dialogue_branch_quest(&stub);
                     break;
                  case dovah::form_type::topic:
                     quest_stub = dovah::form_stub_helpers::get_dialogue_topic_quest(&stub);
                     break;
                  case dovah::form_type::topic_info:
                     quest_stub = dovah::form_stub_helpers::get_dialogue_topic_quest(stub.get_parent_form());
                     break;
               }
               if (quest_stub) {
                  open_edit_dialog_for_form(*quest_stub);
                  auto* dialog = qobject_cast<FormDialogQuest*>(_get_extant_dialog(*quest_stub));
                  if (dialog) {
                     //
                     // Check if there's a modal in the way. If so, abort.
                     //
                     if (auto* modal = QApplication::activeModalWidget()) {
                        for (auto* parent = modal->parentWidget(); parent; parent = parent->parentWidget())
                           if (parent == dialog)
                              return;
                     }
                     //
                     // We're good to go. Set the dialog we've found as the parent for 
                     // the one we want to open, and focus the appropriate content in 
                     // the found dialog's dialogue browser.
                     //
                     parent = dialog;
                     switch (stub.form_type) {
                        case dovah::form_type::dialogue_branch:
                           dialog->focus_dialogue_branch(stub);
                           break;
                        case dovah::form_type::topic:
                           dialog->focus_dialogue_topic(stub);
                           break;
                        case dovah::form_type::topic_info:
                           dialog->focus_dialogue_info(stub);
                           break;
                     }
                  }
               }
            }
            break;
      }
      if (!parent) {
         parent = &MainWindow::get();
      }
   }
   QDialog* opened = nullptr;
   for (auto& pair : factory) {
      if (pair.first == stub.form_type) {
         opened = (pair.second)(stub, parent);
         break;
      }
   }
   if (opened) {
      editor.extant_form_edit_dialogs[stub.formID] = opened;
      QObject::connect(opened, &QDialog::finished, &editor, [formID = stub.formID, opened]() {
         auto& editor = DovahKitCore::get();
         auto& map    = editor.extant_form_edit_dialogs;
         auto  it     = map.find(formID);
         if (it != map.end())
            map.erase(it);
         //
         opened->deleteLater();
      });
      //
      opened->show();
      return;
   }
   //
   auto& info = dovah::form_type_info::lookup(stub.form_type);
   QString title = QObject::tr("Error: cannot edit %1");
   if (&info == &dovah::form_types[0])
      title = title.arg("unknown type");
   else
      title = title.arg(info.name);
   QMessageBox::information(parent, title, QObject::tr("DovahKit doesn't yet support editing this form type."));
}