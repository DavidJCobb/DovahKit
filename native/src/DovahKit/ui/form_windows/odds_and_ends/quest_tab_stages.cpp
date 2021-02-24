#include "quest_tab_stages.h"
#include <QStandardItemModel>
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../dovah/core.h"
#include "../../../editor/core.h"

QuestTabStages::QuestTabStages(dovah::form_stub& s, loaded_t& q, QWidget* parent) : QWidget(parent), stub(s), form(q) {
   ui.setupUi(this);

   #pragma region Stage list
   {
      auto* widget = this->ui.index;
      auto* model  = new QStandardItemModel(widget);
      model->setColumnCount(1);
      widget->setModel(model);
      widget->setUniformItemSizes(true);
      //
      QObject::connect(widget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
         this->_redraw_stage_settings();
         this->_redraw_entry_list();
         this->_redraw_entry_settings();
      });
   }
   #pragma endregion
   //
   // TODO: editing stages
   //
   
   #pragma region Stage flags
   QObject::connect(this->ui.stageFlagStartup, &QCheckBox::stateChanged, this, [this](int state) {
      this->_modify_stage_flag(loaded_t::Stage::flag::startup, state == Qt::CheckState::Checked);
   });
   QObject::connect(this->ui.stageFlagShutdown, &QCheckBox::stateChanged, this, [this](int state) {
      this->_modify_stage_flag(loaded_t::Stage::flag::shutdown, state == Qt::CheckState::Checked);
   });
   QObject::connect(this->ui.stageFlagKeepInstanceData, &QCheckBox::stateChanged, this, [this](int state) {
      this->_modify_stage_flag(loaded_t::Stage::flag::keep_instance_data, state == Qt::CheckState::Checked);
   });
   #pragma endregion
   
   #pragma region Log entry list
   {
      auto* widget = this->ui.logEntries;
      auto* model  = new QStandardItemModel(widget);
      model->setColumnCount(2);
      model->setHeaderData(0, Qt::Orientation::Horizontal, tr("Journal Text"), Qt::DisplayRole);
      model->setHeaderData(1, Qt::Orientation::Horizontal, tr("Conditions"), Qt::DisplayRole);
      widget->setModel(model);
      //
      QObject::connect(widget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
         this->_redraw_entry_settings();
      });
   }
   #pragma endregion
   //
   // TODO: editing log entries
   //

   #pragma region Log entries
      #pragma region Entry settings
      this->ui.logEntryNextQuest->setAllowedFormType(dovah::form_type::quest);
      this->ui.logEntryNextQuest->populate();
      //
      QObject::connect(this->ui.logEntryText, &QPlainTextEdit::textChanged, this, [this]() {
         if (auto* entry = this->_get_log_entry())
            entry->journal_text = this->ui.logEntryText->toPlainText().toStdString();
      });
      QObject::connect(this->ui.logEntryFlagComplete, &QCheckBox::stateChanged, this, [this](int state) {
         if (auto* entry = this->_get_log_entry())
            cobb::edit_bit(entry->flags, loaded_t::LogEntry::flag::complete, state == Qt::CheckState::Checked);
      });
      QObject::connect(this->ui.logEntryFlagFail, &QCheckBox::stateChanged, this, [this](int state) {
         if (auto* entry = this->_get_log_entry())
            cobb::edit_bit(entry->flags, loaded_t::LogEntry::flag::fail, state == Qt::CheckState::Checked);
      });
      #pragma endregion
      #pragma region Fragment
         //
         // TODO
         //
      #pragma endregion
   #pragma endregion

   this->_redraw_stage_list();

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &QuestTabStages::deactivate);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub == &this->stub)
         this->deactivate();
   });
}

void QuestTabStages::deactivate() {
   QObject::disconnect(this->ui.stageFlagStartup);
   QObject::disconnect(this->ui.stageFlagShutdown);
   QObject::disconnect(this->ui.stageFlagKeepInstanceData);
   QObject::disconnect(this->ui.logEntries);
   QObject::disconnect(this->ui.logEntryText);
   QObject::disconnect(this->ui.logEntryFlagComplete);
   QObject::disconnect(this->ui.logEntryFlagFail);
   QObject::disconnect(this->ui.logEntryNextQuest);
   this->ui.logEntryConditions->model()->clearTarget();
   //
   // TODO: disconnect the condition list and the fragment editor
   //
}

QuestTabStages::loaded_t::Stage* QuestTabStages::_get_stage() const noexcept {
   return this->_get_stage(this->_selected_stage_index());
}
QuestTabStages::loaded_t::Stage* QuestTabStages::_get_stage(int id) const noexcept {
   if (id < 0)
      return nullptr;
   for (auto& s : this->form.stages)
      if (s.index == id)
         return &s;
   return nullptr;
}
QuestTabStages::loaded_t::LogEntry* QuestTabStages::_get_log_entry() const noexcept {
   if (auto* s = this->_get_stage()) {
      auto  i    = this->_selected_log_entry_index();
      auto& list = s->entries;
      if (i < 0 || i >= list.size())
         return nullptr;
      return &list[i];
   }
   return nullptr;
}
QuestTabStages::loaded_t::LogEntry* QuestTabStages::_get_log_entry(int stage, int entry) const noexcept {
   auto* s = this->_get_stage(stage);
   if (!s)
      return nullptr;
   auto& list = s->entries;
   if (entry < 0 || entry >= list.size())
      return nullptr;
   return &list[entry];
}

int QuestTabStages::_selected_stage_index() const noexcept {
   auto* widget = this->ui.index;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return -1;
   auto rows = sm->selectedRows();
   if (rows.isEmpty())
      return -1;
   return rows[0].row();
}
int QuestTabStages::_selected_log_entry_index() const noexcept {
   auto* widget = this->ui.logEntries;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return -1;
   auto rows = sm->selectedRows();
   if (rows.isEmpty())
      return -1;
   return rows[0].row();
}

void QuestTabStages::_modify_stage_flag(loaded_t::Stage::flags_t f, bool e) const noexcept {
   if (auto* stage = this->_get_stage())
      cobb::edit_bit(stage->flags, f, e);
}
void QuestTabStages::_modify_log_entry_flag(loaded_t::LogEntry::flags_t f, bool e) const noexcept {
   if (auto* entry = this->_get_log_entry())
      cobb::edit_bit(entry->flags, f, e);
}

void QuestTabStages::_redraw_stage_list() {
   int   prior_id = this->_selected_stage_index(); // prior selected stage ID
   auto* widget   = this->ui.index;
   auto  blocker  = QSignalBlocker(widget);
   auto* model    = (QStandardItemModel*) widget->model();
   assert(model);
   model->clear();
   //
   auto& list = this->form.stages;
   QStandardItem* prior = nullptr;
   for (auto& stage : list) {
      auto* item = new QStandardItem;
      item->setData((int)stage.index);
      item->setText(QString::number(stage.index));
      item->setTextAlignment(Qt::AlignRight);
      model->appendRow(item);
      //
      if (stage.index == prior_id)
         prior = item;
   }
   model->sort(0);
   //
   if (prior) {
      widget->setCurrentIndex(prior->index());
   }
}
void QuestTabStages::_redraw_stage_settings() {
   auto* ptr = this->_get_stage();
   this->ui.stageFlagStartup->setEnabled(ptr != nullptr);
   this->ui.stageFlagShutdown->setEnabled(ptr != nullptr);
   this->ui.stageFlagKeepInstanceData->setEnabled(ptr != nullptr);
   this->ui.logEntries->setEnabled(ptr != nullptr);
   //
   const auto blocker0 = QSignalBlocker(this->ui.stageFlagStartup);
   const auto blocker1 = QSignalBlocker(this->ui.stageFlagShutdown);
   const auto blocker2 = QSignalBlocker(this->ui.stageFlagKeepInstanceData);
   if (!ptr) {
      this->ui.stageFlagStartup->setChecked(false);
      this->ui.stageFlagShutdown->setChecked(false);
      this->ui.stageFlagKeepInstanceData->setChecked(false);
      return;
   }
   this->ui.stageFlagStartup->setChecked(ptr->flags & loaded_t::Stage::flag::startup);
   this->ui.stageFlagShutdown->setChecked(ptr->flags & loaded_t::Stage::flag::shutdown);
   this->ui.stageFlagKeepInstanceData->setChecked(ptr->flags & loaded_t::Stage::flag::keep_instance_data);
}
void QuestTabStages::_redraw_entry_list() {
   int   index    = this->_selected_log_entry_index();
   auto* widget   = this->ui.logEntries;
   auto  blocker  = QSignalBlocker(widget);
   auto* model    = (QStandardItemModel*) widget->model();
   assert(model);
   model->clear();
   //
   auto* ptr = this->_get_stage();
   if (!ptr)
      return;
   //
   auto& list = ptr->entries;
   auto  size = list.size();
   QStandardItem* prior = nullptr;
   for (size_t i = 0; i < size; ++i) {
      auto& entry = list[i];
      auto* item  = new QStandardItem;
      item->setText(entry.journal_text.c_str()); // TODO: column 0 is journal text, column 1 is conditions
      model->appendRow(item);
      //
      if (i == index)
         prior = item;
   }
   //
   if (prior) {
      widget->setCurrentIndex(prior->index());
   }
}
void QuestTabStages::_redraw_entry_settings() {
   auto* ptr = this->_get_log_entry();
   this->ui.logEntryFlagComplete->setEnabled(ptr != nullptr);
   this->ui.logEntryFlagFail->setEnabled(ptr != nullptr);
   this->ui.logEntryNextQuest->setEnabled(ptr != nullptr);
   this->ui.logEntryText->setEnabled(ptr != nullptr);
   if (!ptr) {
      this->ui.logEntryConditions->model()->clearTarget();
      //
      // TODO: Papyrus fragment
      //
      return;
   }
   //
   this->ui.logEntryConditions->model()->setTarget(this->stub, ptr->conditions, true);
   //
   // TODO: Papyrus fragment
   //
}