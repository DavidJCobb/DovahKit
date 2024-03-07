#include "./DKQuestAliasPicker.h"
#include <optional>
#include <QGridLayout>
#include <QLabel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/Quest.h"
   #include "dovah/form_stub.h"
#endif

namespace {
   #if !defined(QT_DESIGNER_LIB)
   bool _test_alias_type(DKQuestAliasPicker::AliasTypes allowed, dovah::loaded_forms::Alias::alias_type type) {
      switch (type) {
         using enum dovah::loaded_forms::Alias::alias_type;
         case location:
            return allowed.testFlag(DKQuestAliasPicker::AliasType::Location);
         case reference:
            return allowed.testFlag(DKQuestAliasPicker::AliasType::Reference);
      }
      return false;
   }
   #endif
}

DKQuestAliasPicker::DKQuestAliasPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);

   this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

   int row = 0;
   {
      auto* label  = new QLabel(tr("Pick Quest:"), this);
      #if !defined(QT_DESIGNER_LIB)
      auto* widget = new FormPicker(this);
      widget->setAllowNone(true);
      widget->setAllowedFormType(dovah::form_type::quest);
      //
      this->_subwidgets.quest = widget;
      #else
      auto* widget = new QComboBox(this);
      this->_subwidgets.fake_formpicker = widget;
      #endif
      label->setBuddy(widget);

      layout->addWidget(label,  row, 0);
      layout->addWidget(widget, row, 1);
   }
   ++row;
   {
      auto* label  = new QLabel(tr("Pick Alias:"), this);
      auto* widget = new QComboBox(this);
      label->setBuddy(widget);
      //
      this->_subwidgets.alias = widget;

      layout->addWidget(label, row, 0);
      layout->addWidget(widget, row, 1);
   }
   layout->setColumnStretch(0, 0);
   layout->setColumnStretch(1, 1);

   #pragma region Tab order
   {
      auto* quest_picker =
         #if !defined(QT_DESIGNER_LIB)
            this->_subwidgets.quest
         #else
            this->_subwidgets.fake_formpicker
         #endif
      ;
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(quest_picker);
      this->setTabOrder(quest_picker, this->_subwidgets.alias);
   }
   #pragma endregion

   #if !defined(QT_DESIGNER_LIB)
   QObject::connect(this->_subwidgets.quest, &FormPicker::formChanged, this, [this](dovah::form_stub* quest) {
      this->setQuest(quest);
   });
   QObject::connect(this->_subwidgets.alias, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      emit this->aliasChanged(this->questAlias());
   });
   #endif
}

DKQuestAliasPicker::AliasTypes DKQuestAliasPicker::allowedTypes() const {
   return this->_state.allowed_types;
}
void DKQuestAliasPicker::setAllowedTypes(AliasTypes types) {
   if (types == this->allowedTypes())
      return;
   this->_state.allowed_types = types;
   this->_updateAliasList(true);
}

#if !defined(QT_DESIGNER_LIB)
ui::types::quest_alias DKQuestAliasPicker::questAlias() const {
   ui::types::quest_alias out;
   out.quest = this->quest();
   if (out.quest) {
      auto data = this->_subwidgets.alias->currentData();
      if (!data.isNull())
         out.alias_id = data.toInt();
   }
   return out;
}
void DKQuestAliasPicker::setQuestAlias(const ui::types::quest_alias& v) {
   auto prior = this->questAlias();
   if (v.empty() && prior.empty())
      return;
   else if (prior == v)
      return;

   if (!v.quest) {
      this->setQuest(nullptr);
      return;
   }

   const auto blocker = QSignalBlocker(this->_subwidgets.alias);

   bool quest_changing = v.quest != prior.quest;
   bool alias_changing = quest_changing;

   if (quest_changing) {
      this->_state.quest      = v.quest;
      this->_state.quest_data = v.quest->load().ptr_cast<dovah::loaded_forms::Quest>();

      this->_subwidgets.alias->clear(); // force a value change
      this->_updateAliasList(false);
   }

   auto i = this->_subwidgets.alias->findData(v.alias_id);
   if (i >= 0) {
      this->_subwidgets.alias->setCurrentIndex(i); // will emit a change
      alias_changing = true;
   }

   if (quest_changing) {
      emit this->questChanged(v.quest);
   }
   if (alias_changing) {
      emit this->aliasChanged(this->questAlias());
   }
}

dovah::form_stub* DKQuestAliasPicker::quest() const {
   return this->_state.quest;
}
void DKQuestAliasPicker::setQuest(dovah::form_stub* quest) {
   if (quest == this->quest())
      return;
   if (quest->form_type != dovah::form_type::quest)
      return;
   this->_state.quest = quest;
   if (quest) {
      this->_state.quest_data = quest->load().ptr_cast<dovah::loaded_forms::Quest>();
   } else {
      this->_state.quest_data = {};
   }
   this->_subwidgets.alias->clear(); // force a value change
   this->_updateAliasList(false);
   emit this->questChanged(quest);
   emit this->aliasChanged(this->questAlias());
}
#endif

void DKQuestAliasPicker::_updateAliasList(bool allow_signals) {
   #if !defined(QT_DESIGNER_LIB)
   std::optional<uint16_t> last_alias_id;

   auto* widget = this->_subwidgets.alias;
   {
      auto data = widget->currentData();
      if (!data.isNull())
         last_alias_id = data.toInt();
   }

   const auto blocker = QSignalBlocker(widget);

   widget->setUpdatesEnabled(false);
   widget->clear();

   if (this->_state.quest_data) {
      for (auto* alias : this->_state.quest_data->aliases) {
         if (!_test_alias_type(this->_state.allowed_types, alias->type))
            continue;
         if (alias->id == ui::types::quest_alias::no_alias_id || alias->id > 0xFFFF)
            continue;
         widget->addItem(QString::fromStdString(alias->name), alias->id);
      }
      widget->model()->sort(0);

      if (last_alias_id.has_value()) {
         auto i = widget->findData(last_alias_id.value());
         if (i >= 0) {
            widget->setCurrentIndex(i);
         }
      }
   }

   if (allow_signals) {
      if (last_alias_id.has_value() && last_alias_id.value() != ui::types::quest_alias::no_alias_id) {
         auto data = widget->currentData();
         if (data.isNull() || data.toInt() == ui::types::quest_alias::no_alias_id) {
            emit this->aliasChanged(this->questAlias());
         }
      }
   }

   widget->setUpdatesEnabled(true);
   #endif
}