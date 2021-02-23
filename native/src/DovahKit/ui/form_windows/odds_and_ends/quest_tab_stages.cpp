#include "quest_tab_stages.h"
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../dovah/core.h"
#include "../../../editor/core.h"

QuestTabStages::QuestTabStages(dovah::form_stub& s, loaded_t& q, QWidget* parent) : QWidget(parent), stub(s), form(q) {
   ui.setupUi(this);

   //
   // TODO: selecting, editing stages
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
   
   //
   // TODO: selecting, editing log entries
   //

   #pragma region Log entries
      
      #pragma region Fragment
      #pragma endregion
   #pragma endregion

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