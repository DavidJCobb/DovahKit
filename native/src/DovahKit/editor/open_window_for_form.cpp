#include "open_window_for_form.h"
#include <QMessageBox>
#include "../dovah/form_stub.h"
#include "core.h"
#include "../ui/main_window/form_use_info.h"
#include "../ui/form_windows/activator.h"
#include "../ui/form_windows/actor_base.h"
#include "../ui/form_windows/association_type.h"
#include "../ui/form_windows/cell.h"
#include "../ui/form_windows/class.h"
#include "../ui/form_windows/color.h"
#include "../ui/form_windows/container.h"
#include "../ui/form_windows/door.h"
#include "../ui/form_windows/equip_slot.h"
#include "../ui/form_windows/faction.h"
#include "../ui/form_windows/flora.h"
#include "../ui/form_windows/formlist.h"
#include "../ui/form_windows/global.h"
#include "../ui/form_windows/head_part.h"
#include "../ui/form_windows/landtexture.h"
#include "../ui/form_windows/leveled_character.h"
#include "../ui/form_windows/leveled_item.h"
#include "../ui/form_windows/leveled_spell.h"
#include "../ui/form_windows/light.h"
#include "../ui/form_windows/misc_item.h"
#include "../ui/form_windows/movement_type.h"
#include "../ui/form_windows/note.h"
#include "../ui/form_windows/outfit.h"
#include "../ui/form_windows/quest.h"
#include "../ui/form_windows/relationship.h"
#include "../ui/form_windows/shout.h"
#include "../ui/form_windows/static.h"
#include "../ui/form_windows/textureset.h"
#include "../ui/form_windows/voicetype.h"
#include "../ui/form_windows/word_of_power.h"
#include "ui/main_window.h" // MainWindow::get

namespace {
   template<typename T> QDialog* _make(dovah::form_stub& f, QWidget* p) {
      return new T(f, p);
   }

   constexpr std::array factory = {
      std::pair{ dovah::form_type::activator,         _make<FormDialogActivator> },
      std::pair{ dovah::form_type::actor_base,        _make<FormDialogActorBase> },
      std::pair{ dovah::form_type::association_type,  _make<FormDialogAssociationType> },
      std::pair{ dovah::form_type::cell,              _make<FormDialogCell> },
      std::pair{ dovah::form_type::combat_class,      _make<FormDialogClass> },
      std::pair{ dovah::form_type::color,             _make<FormDialogColor> },
      std::pair{ dovah::form_type::container,         _make<FormDialogContainer> },
      std::pair{ dovah::form_type::door,              _make<FormDialogDoor> },
      std::pair{ dovah::form_type::equip_slot,        _make<FormDialogEquipSlot> },
      std::pair{ dovah::form_type::faction,           _make<FormDialogFaction> },
      std::pair{ dovah::form_type::flora,             _make<FormDialogFlora> },
      std::pair{ dovah::form_type::formlist,          _make<FormDialogFormList> },
      std::pair{ dovah::form_type::global,            _make<FormDialogGlobal> },
      std::pair{ dovah::form_type::head_part,         _make<FormDialogHeadPart> },
      std::pair{ dovah::form_type::land_texture,      _make<FormDialogLandTexture> },
      std::pair{ dovah::form_type::leveled_character, _make<FormDialogLeveledCharacter> },
      std::pair{ dovah::form_type::leveled_item,      _make<FormDialogLeveledItem> },
      std::pair{ dovah::form_type::leveled_spell,     _make<FormDialogLeveledSpell> },
      std::pair{ dovah::form_type::light,             _make<FormDialogLight> },
      std::pair{ dovah::form_type::misc_item,         _make<FormDialogMiscItem> },
      std::pair{ dovah::form_type::movement_type,     _make<FormDialogMovementType> },
      std::pair{ dovah::form_type::note,              _make<FormDialogNote> },
      std::pair{ dovah::form_type::outfit,            _make<FormDialogOutfit> },
      std::pair{ dovah::form_type::quest,             _make<FormDialogQuest> },
      std::pair{ dovah::form_type::relationship,      _make<FormDialogRelationship> },
      std::pair{ dovah::form_type::shout,             _make<FormDialogShout> },
      std::pair{ dovah::form_type::statik,            _make<FormDialogStatic> },
      std::pair{ dovah::form_type::texture_set,       _make<FormDialogTextureSet> },
      std::pair{ dovah::form_type::voicetype,         _make<FormDialogVoicetype> },
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
   //
   // First, let's check if there's already a window for this form. If so, we should just 
   // refocus that window instead of opening a new one.
   //
   auto  formID = stub.formID;
   auto& editor = DovahKitCore::get();
   auto  it     = editor.extant_form_edit_dialogs.find(formID);
   if (it != editor.extant_form_edit_dialogs.end()) {
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
   if (parent == nullptr) {
      //
      // TODO: If this is a child form (e.g. DIAL, INFO), reuse or open windows for its 
      // ancestor forms (e.g. QUST, DIAL) and use the appropriate one as the parent.
      //
      parent = &MainWindow::get();
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
      QObject::connect(opened, &QDialog::finished, &editor, [formID, opened]() {
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