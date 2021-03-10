#include "quest_tab_stages.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "../../generic/PapyrusFragmentEditor.h"
#include "../../generic/QStandardItemModelDKEx.h" // enhanced QStandardItemModel
#include "../../../helpers/qt/basic_bindings.h"
#include "../../../dovah/core.h"
#include "../../../editor/core.h"
#include "../../../editor/helpers/stringify_conditions.h"

namespace {
   constexpr int no_stage = -1;
   static_assert(std::numeric_limits<decltype(dovah::loaded_forms::Quest::Stage::index)>::min() > no_stage, "The sentinel value that this UI uses for \"no stage\" needs to be outside of the range of valid stage IDs.");
}

QuestTabStages::QuestTabStages(dovah::form_stub& s, loaded_t& q, QWidget* parent) : QWidget(parent), stub(s), form(q) {
   ui.setupUi(this);

   #pragma region Stage list
      {
         auto* widget = this->ui.index;
         auto* model  = new QStandardItemModelDKEx(widget);
         model->setColumnCount(1);
         model->setSortRole(Qt::UserRole);
         widget->setModel(model);
         widget->setUniformItemSizes(true);
         //
         QObject::connect(widget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            this->_redraw_stage_settings();
            this->_redraw_entry_list();
            this->_redraw_entry_settings();
         });
      }
      #pragma region Context menu
      this->context_menu_actions.stage_list.insert = new QAction(tr("New..."), this->ui.index);
      this->context_menu_actions.stage_list.remove = new QAction(tr("Delete"), this->ui.index);
      //
      QObject::connect(this->context_menu_actions.stage_list.insert, &QAction::triggered, this, [this]() {
         int initial = this->_selected_stage_index();
         if (initial >= 0)
            ++initial;
         else
            initial = 0;
         //
         bool ok;
         int value = QInputDialog::getInt(
            this,
            tr("Create quest stage"),
            tr("What number should this quest stage use? The value must be unique."),
            initial,
            std::numeric_limits<decltype(loaded_t::Stage::index)>::min(),
            std::numeric_limits<decltype(loaded_t::Stage::index)>::max(),
            1,  // step
            &ok // set to (true) if the user clicked OK
         );
         if (!ok)
            return;
         //
         if (auto* existing = this->_get_stage(value)) {
            QMessageBox::critical(
               this,
               tr("Error"),
               tr("This quest already has a stage with ID number %1.").arg(value)
            );
            return;
         }
         if (this->form.insert_stage(value)) {
            this->_redraw_stage_list();
            this->_select_stage(value);
         }
      });
      QObject::connect(this->context_menu_actions.stage_list.remove, &QAction::triggered, this, [this]() {
         int prev = std::numeric_limits<int>::min();
         int next = std::numeric_limits<int>::max();
         int sel  = this->_selected_stage_index();
         if (sel < 0) // exit if no selection
            return;
         for (auto& stage : this->form.stages) {
            auto id = stage.index;
            if (id < sel) {
               if (id > prev)
                  prev = id;
            } else if (id > sel) {
               if (id < next)
                  next = id;
            }
         }
         this->form.remove_stage(sel);
         if (next >= 0)
            this->_select_stage(next);
         else
            this->_select_stage(prev);
         this->_redraw_stage_list();
      });
      //
      this->ui.index->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(this->ui.index, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
         auto* opener = this->ui.index;
         auto* sm     = opener->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         //
         this->context_menu_actions.stage_list.remove->setVisible(!rows.isEmpty());
         //
         QMenu menu(opener);
         menu.addAction(this->context_menu_actions.stage_list.insert);
         menu.addAction(this->context_menu_actions.stage_list.remove);
         //
         if (menu.isEmpty())
            return; // don't show a menu if all of its contents are disabled or hidden
         menu.exec(opener->mapToGlobal(pos));
      });
      #pragma endregion
   #pragma endregion
   
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
         auto* model  = new QStandardItemModelDKEx(widget);
         model->setAutoTooltips(true); // QStandardItemModelDKEx
         model->setColumnCount(2);
         widget->setModel(model);
         model->setHorizontalHeaderLabels({ tr("Journal Text"), tr("Conditions") });
         widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
         widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
         //
         auto* header = widget->horizontalHeader();
         header->setDefaultAlignment(Qt::AlignLeft);
         header->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
         header->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
         header->setSortIndicatorShown(false);
         header->setStretchLastSection(true);
         {
            auto* vh = widget->verticalHeader();
            vh->setVisible(false);
            vh->setDefaultSectionSize(vh->minimumSectionSize());
         }
         //
         QObject::connect(widget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            this->_redraw_entry_settings();
         });
         //
         // Redraw log entries in the table if the fields we list in the table are edited:
         //
         QObject::connect(this->ui.logEntryConditions, &ConditionList::conditionEdited, this, &QuestTabStages::redrawEntryListSelectedItem);
      }
      #pragma region Context menu
      this->context_menu_actions.log_entry_list.insert   = new QAction(tr("New..."), this->ui.logEntries);
      this->context_menu_actions.log_entry_list.remove   = new QAction(tr("Delete"), this->ui.logEntries);
      this->context_menu_actions.log_entry_list.moveUp   = new QAction(tr("Move up"), this->ui.logEntries);
      this->context_menu_actions.log_entry_list.moveDown = new QAction(tr("Move down"), this->ui.logEntries);
      //
      QObject::connect(this->context_menu_actions.log_entry_list.insert, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_stage();
         if (!stage)
            return;
         auto  index = stage->entries.size();
         stage->entries.emplace_back();
         this->_redraw_entry_list();
         this->_select_log_entry(index);
      });
      QObject::connect(this->context_menu_actions.log_entry_list.remove, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_stage();
         if (!stage)
            return;
         int s = this->_selected_stage_index();
         int e = this->_selected_log_entry_index();
         this->form.remove_log_entry(s, e);
         this->_redraw_entry_list();
         //
         if (e < stage->entries.size())
            this->_select_log_entry(e);
         else if (e > 0)
            this->_select_log_entry(e - 1);
      });
      QObject::connect(this->context_menu_actions.log_entry_list.moveUp, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_stage();
         if (!stage)
            return;
         auto& list = stage->entries;
         int   e    = this->_selected_log_entry_index();
         if (e <= 0)
            return;
         std::swap(list[e], list[e - 1]);
         this->_redraw_entry_list();
         this->_select_log_entry(e - 1);
      });
      QObject::connect(this->context_menu_actions.log_entry_list.moveDown, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_stage();
         if (!stage)
            return;
         auto& list = stage->entries;
         int   e    = this->_selected_log_entry_index();
         if (e >= list.size() - 1)
            return;
         std::swap(list[e], list[e + 1]);
         this->_redraw_entry_list();
         this->_select_log_entry(e + 1);
      });
      //
      this->ui.logEntries->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(this->ui.logEntries, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
         auto* opener = this->ui.logEntries;
         auto* sm     = opener->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         bool any  = !rows.isEmpty();
         //
         this->context_menu_actions.log_entry_list.moveUp->setVisible(any);
         this->context_menu_actions.log_entry_list.moveDown->setVisible(any);
         this->context_menu_actions.log_entry_list.remove->setVisible(any);
         if (any) {
            int   row   = rows[0].row();
            auto* stage = this->_get_stage();
            this->context_menu_actions.log_entry_list.moveUp->setEnabled(row > 0);
            this->context_menu_actions.log_entry_list.moveDown->setEnabled(stage && row + 1 < stage->entries.size());
         }
         //
         QMenu menu(opener);
         menu.addAction(this->context_menu_actions.log_entry_list.insert);
         menu.addAction(this->context_menu_actions.log_entry_list.moveUp);
         menu.addAction(this->context_menu_actions.log_entry_list.moveDown);
         menu.addAction(this->context_menu_actions.log_entry_list.remove);
         //
         if (menu.isEmpty())
            return; // don't show a menu if all of its contents are disabled or hidden
         menu.exec(opener->mapToGlobal(pos));
      });
      #pragma endregion
   #pragma endregion

   #pragma region Log entries
      #pragma region Entry settings
      this->ui.logEntryNextQuest->setAllowedFormType(dovah::form_type::quest);
      this->ui.logEntryNextQuest->populate();
      //
      QObject::connect(this->ui.logEntryText, &QPlainTextEdit::textChanged, this, [this]() {
         if (auto* entry = this->_get_log_entry()) {
            entry->journal_text = this->ui.logEntryText->toPlainText().toStdString();
            this->redrawEntryListSelectedItemText();
         }
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
         {
            PapyrusFragmentEditor* widget = this->ui.logEntryFragment;
            widget->setScriptnameMaxLength(std::numeric_limits<uint16_t>::max());
            widget->setFunctionMaxLength(std::numeric_limits<uint16_t>::max());
            auto& papyrus = this->form.script_data;
            for (auto& script : papyrus.scripts)
               widget->addScriptname(script.name.c_str());
            //
            QObject::connect(widget, &PapyrusFragmentEditor::currentScriptnameChanged, this, [this](const QString& name) {
               auto* lo = this->_get_log_entry();
               if (lo) 
                  lo->fragment.filename = name.toStdString().c_str();
            });
            QObject::connect(widget, &PapyrusFragmentEditor::currentFunctionChanged, this, [this](const QString& name) {
               auto* lo = this->_get_log_entry();
               if (lo)
                  lo->fragment.function = name.toStdString().c_str();
            });
         }
      #pragma endregion
   #pragma endregion

   this->_redraw_stage_list();
   this->_redraw_stage_settings();
   this->_redraw_entry_list();
   this->_redraw_entry_settings();

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
void QuestTabStages::redrawEntryListSelectedItem() {
   loaded_t::LogEntry* entry = nullptr;
   //
   auto* widget = this->ui.logEntries;
   auto* sm     = widget->selectionModel();
   auto* model  = (QStandardItemModelDKEx*) widget->model();
   if (!sm || !model)
      return;
   auto index = sm->currentIndex();
   if (!index.isValid())
      return;
   auto row = index.row();
   //
   if (auto* s = this->_get_stage()) {
      auto& list = s->entries;
      if (row < 0 || row >= list.size())
         return;
      entry = &list[row];
   }
   if (!entry)
      return;
   //
   auto* col0 = model->item(row, 0);
   auto* col1 = model->item(row, 1);
   if (!col0 || !col1)
      return;
   auto  ctx  = dovah::loaded_forms::components::condition_context(this->stub, true);
   col0->setText(entry->journal_text.c_str());
   col1->setText(editor_helpers::stringify_condition_list(entry->conditions, ctx));
}
void QuestTabStages::redrawEntryListSelectedItemText() {
   loaded_t::LogEntry* entry = nullptr;
   //
   auto* widget = this->ui.logEntries;
   auto  index  = widget->currentIndex();
   if (!index.isValid())
      return;
   auto row = index.row();
   //
   if (auto* s = this->_get_stage()) {
      auto& list = s->entries;
      if (row < 0 || row >= list.size())
         return;
      entry = &list[row];
   }
   if (!entry)
      return;
   //
   auto* model = (QStandardItemModelDKEx*)widget->model();
   auto* col0  = model->item(row, 0);
   if (!col0)
      return;
   col0->setText(entry->journal_text.c_str());
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

void QuestTabStages::_select_stage(int id) noexcept {
   auto* widget  = this->ui.index;
   auto* model   = (QStandardItemModelDKEx*) widget->model();
   auto  indices = model->match(
      model->index(0, 0),
      Qt::UserRole,
      id,
      1,
      Qt::MatchExactly
   );
   if (indices.isEmpty()) {
      widget->setCurrentIndex(QModelIndex());
      return;
   }
   widget->setCurrentIndex(indices[0]);
}
void QuestTabStages::_select_log_entry(int index) noexcept {
   auto* widget = this->ui.logEntries;
   auto* model  = (QStandardItemModelDKEx*)widget->model();
   auto  qmi    = model->index(index, 0, QModelIndex());
   widget->setCurrentIndex(qmi);
}

int QuestTabStages::_selected_stage_index() const noexcept {
   auto* widget = this->ui.index;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return no_stage;
   auto index = sm->currentIndex();
   auto data  = widget->model()->data(index, Qt::UserRole);
   if (!data.isValid())
      return no_stage;
   return data.toInt();
}
int QuestTabStages::_selected_log_entry_index() const noexcept {
   auto* widget = this->ui.logEntries;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return -1;
   auto index = sm->currentIndex();
   if (!index.isValid())
      return -1;
   return index.row();
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
   auto* model    = (QStandardItemModelDKEx*) widget->model();
   assert(model);
   model->clearBody();
   //
   auto& list = this->form.stages;
   if (list.empty())
      return;
   QStandardItem* prior = nullptr;
   for (auto& stage : list) {
      auto* item = new QStandardItem;
      item->setData((int)stage.index, Qt::UserRole);
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
   int   index   = this->_selected_log_entry_index();
   auto* widget  = this->ui.logEntries;
   auto  blocker = QSignalBlocker(widget);
   auto* model   = (QStandardItemModelDKEx*) widget->model();
   assert(model);
   model->clearBody();
   //
   auto* ptr = this->_get_stage();
   if (!ptr)
      return;
   //
   auto  ctx  = dovah::loaded_forms::components::condition_context(this->stub, true);
   auto& list = ptr->entries;
   auto  size = list.size();
   QModelIndex prior;
   for (size_t i = 0; i < size; ++i) {
      auto& entry = list[i];
      auto* col0  = new QStandardItem(entry.journal_text.c_str());
      auto* col1  = new QStandardItem(editor_helpers::stringify_condition_list(entry.conditions, ctx));
      model->appendRow({ col0, col1 });
      //
      if (i == index)
         prior = col0->index();
   }
   //
   widget->setCurrentIndex(prior);
}
void QuestTabStages::_redraw_entry_settings() {
   auto* ptr = this->_get_log_entry();
   this->ui.logEntryFlagComplete->setEnabled(ptr != nullptr);
   this->ui.logEntryFlagFail->setEnabled(ptr != nullptr);
   this->ui.logEntryNextQuest->setEnabled(ptr != nullptr);
   this->ui.logEntryText->setEnabled(ptr != nullptr);
   this->ui.logEntryFragment->setEnabled(ptr != nullptr);
   this->ui.logEntryConditions->setEnabled(ptr != nullptr);
   //
   const auto blocker0 = QSignalBlocker(this->ui.logEntryText);
   const auto blocker1 = QSignalBlocker(this->ui.logEntryFragment);
   const auto blocker2 = QSignalBlocker(this->ui.logEntryFlagComplete);
   const auto blocker3 = QSignalBlocker(this->ui.logEntryFlagFail);
   const auto blocker4 = QSignalBlocker(this->ui.logEntryNextQuest);
   //
   if (!ptr) {
      this->ui.logEntryFlagComplete->setChecked(false);
      this->ui.logEntryFlagFail->setChecked(false);
      this->ui.logEntryNextQuest->setFormStub(nullptr);
      this->ui.logEntryConditions->model()->clearTarget();
      this->ui.logEntryText->clear();
      this->ui.logEntryFragment->clearCurrentValues();
      return;
   }
   //
   this->ui.logEntryFlagComplete->setChecked(ptr->flags & loaded_t::LogEntry::flag::complete);
   this->ui.logEntryFlagFail->setChecked(ptr->flags & loaded_t::LogEntry::flag::fail);
   this->ui.logEntryNextQuest->setFormStub(ptr->next_quest_id.get_form_stub());
   this->ui.logEntryText->setPlainText(ptr->journal_text.c_str());
   this->ui.logEntryFragment->setCurrentScriptname(ptr->fragment.filename.c_str());
   this->ui.logEntryFragment->setCurrentFunction(ptr->fragment.function.c_str());
   this->ui.logEntryConditions->model()->setTarget(this->stub, ptr->conditions, true);
}

void QuestTabStages::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   if (this->_did_first_show)
      return;
   this->_did_first_show = true;
   if (auto* header = this->ui.logEntries->horizontalHeader()) {
      auto width = header->width();
      auto third = width / 3;
      header->resizeSection(0, width - third);
      header->resizeSection(1, 0); // set the last section to minimum size and let it stretch; that way, enlarging the prior sections doesn't cause this one to clip out of bounds
   }
}