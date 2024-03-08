#include "./DKQuestAliasPicker.h"
#include <optional>
#include <QGridLayout>
#include <QLabel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/Quest.h"
   #include "dovah/form_stub.h"
   #include "editor/core.h"
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
      auto* widget = new DKFormPicker(this);
      widget->setAllowNone(true);
      widget->setAllowedFormType(dovah::form_type::quest);
      label->setBuddy(widget);
      //
      this->_subwidgets.quest = widget;

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
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_subwidgets.quest);
      this->setTabOrder(this->_subwidgets.quest, this->_subwidgets.alias);
   }
   #pragma endregion

   #if !defined(QT_DESIGNER_LIB)
   QObject::connect(this->_subwidgets.quest, &DKFormPicker::formChanged, this, [this](dovah::form_stub* quest) {
      this->setQuest(quest);
   });
   QObject::connect(this->_subwidgets.alias, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      emit this->aliasChanged(this->questAlias());
   });

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (stub != this->_state.quest)
         return;
      this->_updateAliasList();
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->setQuest(nullptr);
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

QString DKQuestAliasPicker::requiredScriptname() const {
   return QString::fromUtf8(QByteArray::fromStdString(this->_state.required_scriptname));
}
void DKQuestAliasPicker::setRequiredScriptname(QString s) {
   return this->setRequiredScriptname(s.toUtf8().toStdString());
}
void DKQuestAliasPicker::setRequiredScriptname(std::string_view s) {
   if (this->_state.required_scriptname == s)
      return;
   this->_state.required_scriptname = s;

   auto prior = this->questAlias();
   {
      const auto blocker = QSignalBlocker(this);
      this->_subwidgets.quest->setRequiredAliasScriptname(s);
      this->_updateAliasList();
   }
   auto after = this->questAlias();
   if (prior != after) {
      if (prior.quest != after.quest)
         emit this->questChanged(after.quest);
      emit this->aliasChanged(after);
   }
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
   if (quest && quest->form_type != dovah::form_type::quest)
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

   bool empty = true;
   if (this->_state.quest_data) {
      for (auto* alias : this->_state.quest_data->aliases) {
         if (!_test_alias_type(this->_state.allowed_types, alias->type))
            continue;
         if (alias->id == ui::types::quest_alias::no_alias_id || alias->id > 0xFFFF)
            continue;
         if (!this->_state.required_scriptname.empty()) {
            if (alias->script_data.lookup_script(this->_state.required_scriptname) == nullptr)
               continue;
         }
         empty = false;
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
   widget->setDisabled(empty);

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