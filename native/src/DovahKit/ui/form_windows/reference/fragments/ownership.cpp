#include "./ownership.h"
#include <cassert>
#include <QComboBox>
#include <QCoreApplication>
#include <QVariant>
#include "widgets/DKFormPicker.h"
#include "dovah/forms/components/extra_data/types/o/ownership.h"
#include "dovah/forms/components/extra_data/types/r/rank.h"
#include "dovah/forms/Faction.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/core.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void ownership::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;

      controls.owner_form->setAllowedFormTypes({ dovah::form_type::actor_base, dovah::form_type::faction });
      controls.owner_form->setAlwaysSplitTypes(true);

      QObject::connect(controls.owner_form, &DKFormPicker::formChanged, &owner, [this](dovah::form_stub* stub) {
         auto& form = loaded();
         if (stub) {
            form.extra_data.get_or_create<extra_data_types::ownership>()->form.set(form, stub);
         } else {
            form.extra_data.remove<extra_data_types::ownership>(form);
            form.extra_data.remove<extra_data_types::rank>(form);
         }
         this->_update_ownership_rank_picker();
      });
      QObject::connect(&DovahKitCore::get(), &DovahKitCore::formModified, &owner, [this](dovah::form_stub* stub) {
         if (stub != this->controls.owner_form->formStub())
            return;
         this->_update_ownership_rank_picker();
      });
   }
   void ownership::load(loaded_form_type& form) {
      this->stub = &form.stub;
      
      using form_extra_data = extra_data_types::ownership;
      using rank_extra_data = extra_data_types::rank;
      auto* form_picker = this->controls.owner_form;
      auto* rank_picker = this->controls.owner_rank;
      if (const auto* extra = form.extra_data.get<form_extra_data>()) {
         dovah::form_stub* owner = extra->form.get_form_stub();
         form_picker->setFormStub(owner);
         if (owner && owner->form_type == dovah::form_type::faction) {
            rank_picker->setEnabled(true);

            const auto blocker = QSignalBlocker(rank_picker);
            this->_update_ownership_rank_picker();

            if (auto* extra_rank = form.extra_data.get<rank_extra_data>()) {
               auto i = rank_picker->findData(extra_rank->value);
               if (i >= 0) {
                  rank_picker->setCurrentIndex(i);
               } else {
                  rank_picker->setCurrentIndex(rank_picker->findData(rank_extra_data::sentinel_value_for_unset));
                  form.extra_data.remove<rank_extra_data>(form);
               }
            } else {
               rank_picker->setCurrentIndex(rank_picker->findData(rank_extra_data::sentinel_value_for_unset));
            }
         } else {
            rank_picker->setEnabled(false);
         }
      } else {
         form_picker->setFormStub(nullptr);
         rank_picker->setEnabled(false);
      }
   }
   void ownership::save(loaded_form_type& form) {
   }
   
   void ownership::_update_ownership_rank_picker() {
      auto& editor  = DovahKitCore::get();
      auto& working = this->loaded();

      auto* form_picker = this->controls.owner_form;
      auto* rank_picker = this->controls.owner_rank;

      auto* form = form_picker->formStub();
      if (!form || form->form_type != dovah::form_type::faction) {
         rank_picker->clear();
         working.extra_data.remove<extra_data_types::rank>(working);
         return;
      }

      int prior_value = extra_data_types::rank::sentinel_value_for_unset;
      {
         auto data = rank_picker->currentData();
         if (data.isValid())
            prior_value = data.toInt();
      }
      const auto blocker = QSignalBlocker(rank_picker);
      rank_picker->clear();
      rank_picker->addItem(QCoreApplication::translate("faction rank text", "Any Rank", "ownership: no faction rank"), -1);

      auto loaded = form->load().ptr_cast<dovah::loaded_forms::Faction>();
      if (!loaded) {
         working.extra_data.remove<extra_data_types::rank>(working);
         return;
      }
      for (auto& rank : loaded->ranks) {
         QString text;
         {
            auto masc = editor.convert_localized_string(rank.title_masc);
            auto fem = editor.convert_localized_string(rank.title_fem);
            if (masc == fem) {
               text = fem;
            } else {
               text = QCoreApplication::translate("faction rank text", "%1 / %2", "ownership: faction rank names").arg(masc).arg(fem);
            }
         }
         rank_picker->addItem(text, rank.id);
      }
      auto i = rank_picker->findData(prior_value);
      if (i >= 0) {
         rank_picker->setCurrentIndex(i);
      } else {
         rank_picker->setCurrentIndex(0);
         working.extra_data.remove<extra_data_types::rank>(working);
      }
   }

   ownership::loaded_form_type& ownership::loaded() {
      assert(this->stub != nullptr);
      return *this->stub->get_working_copy<loaded_form_type>();
   }
}