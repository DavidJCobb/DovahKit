#include "quest_tab_objectives.h"
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

QuestTabObjectives::QuestTabObjectives(dovah::form_stub& s, loaded_t& q, QWidget* parent) : QWidget(parent), stub(s), form(q) {
   ui.setupUi(this);

   #pragma region Stage list
      {
         auto* widget = this->ui.objectives;
         auto* model  = new QStandardItemModelDKEx(widget);
         model->setColumnCount(2);
         model->setSortRole(Qt::UserRole);
         widget->setModel(model);
         widget->setUniformItemSizes(true);
         //
         QObject::connect(widget->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            this->_redraw_objective_settings();
            this->_redraw_target_list();
            this->_redraw_target_settings();
         });
      }
      #pragma region Context menu
      this->context_menu_actions.objective_list.insert = new QAction(tr("New..."), this->ui.objectives);
      this->context_menu_actions.objective_list.remove = new QAction(tr("Delete"), this->ui.objectives);
      //
      QObject::connect(this->context_menu_actions.objective_list.insert, &QAction::triggered, this, [this]() {
         int initial = this->_selected_objective_id();
         if (initial >= 0)
            ++initial;
         else
            initial = 0;
         //
         bool ok;
         int value = QInputDialog::getInt(
            this,
            tr("Create quest objective"),
            tr("What ID number should this quest objective use? The value must be unique and between 0 and 65535, inclusive."),
            initial,
            std::numeric_limits<decltype(loaded_t::Objective::index)>::min(),
            std::numeric_limits<decltype(loaded_t::Objective::index)>::max(),
            1,  // step
            &ok // set to (true) if the user clicked OK
         );
         if (!ok)
            return;
         //
         if (auto* existing = this->_get_objective(value)) {
            QMessageBox::critical(
               this,
               tr("Error"),
               tr("This quest already has a objective with ID number %1.").arg(value)
            );
            return;
         }
         if (this->form.insert_stage(value)) {
            this->_redraw_objective_list();
            this->_select_objective(value);
         }
      });
      QObject::connect(this->context_menu_actions.objective_list.remove, &QAction::triggered, this, [this]() {
         int prev = std::numeric_limits<int>::min();
         int next = std::numeric_limits<int>::max();
         int sel  = this->_selected_objective_id();
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
            this->_select_objective(next);
         else
            this->_select_objective(prev);
         this->_redraw_objective_list();
      });
      //
      this->ui.objectives->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(this->ui.objectives, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
         auto* opener = this->ui.objectives;
         auto* sm     = opener->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         //
         this->context_menu_actions.objective_list.remove->setVisible(!rows.isEmpty());
         //
         QMenu menu(opener);
         menu.addAction(this->context_menu_actions.objective_list.insert);
         menu.addAction(this->context_menu_actions.objective_list.remove);
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
         auto* widget = this->ui.targets;
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
            this->_redraw_target_settings();
         });
         //
         // Redraw log entries in the table if the fields we list in the table are edited:
         //
         QObject::connect(this->ui.targetConditions, &ConditionList::conditionEdited, this, &QuestTabObjectives::redrawTargetListSelectedItemConditions);
      }
      #pragma region Context menu
      this->context_menu_actions.target_list.insert   = new QAction(tr("New..."),    this->ui.targets);
      this->context_menu_actions.target_list.remove   = new QAction(tr("Delete"),    this->ui.targets);
      this->context_menu_actions.target_list.moveUp   = new QAction(tr("Move up"),   this->ui.targets);
      this->context_menu_actions.target_list.moveDown = new QAction(tr("Move down"), this->ui.targets);
      //
      QObject::connect(this->context_menu_actions.target_list.insert, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_objective();
         if (!stage)
            return;
         auto  index = stage->entries.size();
         stage->entries.emplace_back();
         this->_redraw_target_list();
         this->_select_target(index);
      });
      QObject::connect(this->context_menu_actions.target_list.remove, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_objective();
         if (!stage)
            return;
         int s = this->_selected_objective_id();
         int e = this->_selected_target_index();
         //this->form.remove_log_entry(s, e);
         static_assert(false, "remove the objective");
         this->_redraw_target_list();
         //
         if (e < stage->entries.size())
            this->_select_target(e);
         else if (e > 0)
            this->_select_target(e - 1);
      });
      QObject::connect(this->context_menu_actions.target_list.moveUp, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_objective();
         if (!stage)
            return;
         auto& list = stage->entries;
         int   e    = this->_selected_target_index();
         if (e <= 0)
            return;
         std::swap(list[e], list[e - 1]);
         this->_redraw_target_list();
         this->_select_target(e - 1);
      });
      QObject::connect(this->context_menu_actions.target_list.moveDown, &QAction::triggered, this, [this]() {
         auto* stage = this->_get_objective();
         if (!stage)
            return;
         auto& list = stage->entries;
         int   e    = this->_selected_target_index();
         if (e >= list.size() - 1)
            return;
         std::swap(list[e], list[e + 1]);
         this->_redraw_target_list();
         this->_select_target(e + 1);
      });
      //
      this->ui.targets->setContextMenuPolicy(Qt::CustomContextMenu);
      QObject::connect(this->ui.targets, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
         auto* opener = this->ui.targets;
         auto* sm     = opener->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         bool any  = !rows.isEmpty();
         //
         this->context_menu_actions.target_list.moveUp->setVisible(any);
         this->context_menu_actions.target_list.moveDown->setVisible(any);
         this->context_menu_actions.target_list.remove->setVisible(any);
         if (any) {
            int   row   = rows[0].row();
            auto* stage = this->_get_objective();
            this->context_menu_actions.target_list.moveUp->setEnabled(row > 0);
            this->context_menu_actions.target_list.moveDown->setEnabled(stage && row + 1 < stage->entries.size());
         }
         //
         QMenu menu(opener);
         menu.addAction(this->context_menu_actions.target_list.insert);
         menu.addAction(this->context_menu_actions.target_list.moveUp);
         menu.addAction(this->context_menu_actions.target_list.moveDown);
         menu.addAction(this->context_menu_actions.target_list.remove);
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
      this->ui.logEntryNextQuest->setAllowNone(true);
      this->ui.logEntryNextQuest->populate();
      QObject::connect(this->ui.logEntryNextQuest, &FormsOfTypeCombobox::formChanged, this, [this](dovah::form_stub* selected) {
         if (auto* entry = this->_get_log_entry())
            entry->next_quest_id.set(this->form, selected);
      });
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
   #pragma endregion

   this->_redraw_objective_list();
   this->_redraw_objective_settings();
   this->_redraw_target_list();
   this->_redraw_target_settings();

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &QuestTabObjectives::deactivate);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub == &this->stub)
         this->deactivate();
   });
}

void QuestTabObjectives::deactivate() {
   QObject::disconnect(this->ui.stageFlagStartup);
   QObject::disconnect(this->ui.stageFlagShutdown);
   QObject::disconnect(this->ui.stageFlagKeepInstanceData);
   QObject::disconnect(this->ui.logEntries);
   QObject::disconnect(this->ui.logEntryText);
   QObject::disconnect(this->ui.logEntryFlagComplete);
   QObject::disconnect(this->ui.logEntryFlagFail);
   QObject::disconnect(this->ui.logEntryNextQuest);
   this->ui.targetConditions->model()->clearTarget();
   //
   // TODO: disconnect the condition list and the fragment editor
   //
}
void QuestTabObjectives::redrawTargetListSelectedItemConditions() {
   loaded_t::Target* entry = nullptr;
   //
   auto* widget = this->ui.targets;
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
   auto* col1 = model->item(row, 1);
   if (!col1)
      return;
   auto  ctx  = dovah::loaded_forms::components::condition_context(this->stub, true);
   col1->setText(editor_helpers::stringify_condition_list(entry->conditions, ctx));
}
void QuestTabObjectives::redrawTargetListSelectedItemText() {
   loaded_t::Target* entry = nullptr;
   //
   auto* widget = this->ui.targets;
   auto  index  = widget->currentIndex();
   if (!index.isValid())
      return;
   auto row = index.row();
   //
   if (auto* s = this->_get_objective()) {
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

QuestTabObjectives::loaded_t::Objective* QuestTabObjectives::_get_objective() const noexcept {
   return this->_get_objective(this->_selected_objective_id());
}
QuestTabObjectives::loaded_t::Objective* QuestTabObjectives::_get_objective(int id) const noexcept {
   if (id < 0)
      return nullptr;
   for (auto& s : this->form.stages)
      if (s.index == id)
         return &s;
   return nullptr;
}
QuestTabObjectives::loaded_t::Target* QuestTabObjectives::_get_target() const noexcept {
   if (auto* s = this->_get_objective()) {
      auto  i    = this->_selected_target_index();
      auto& list = s->entries;
      if (i < 0 || i >= list.size())
         return nullptr;
      return &list[i];
   }
   return nullptr;
}
QuestTabObjectives::loaded_t::Target* QuestTabObjectives::_get_target(int stage, int entry) const noexcept {
   auto* s = this->_get_objective(stage);
   if (!s)
      return nullptr;
   auto& list = s->entries;
   if (entry < 0 || entry >= list.size())
      return nullptr;
   return &list[entry];
}

void QuestTabObjectives::_select_objective(int id) noexcept {
   auto* widget  = this->ui.objectives;
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
void QuestTabObjectives::_select_target(int index) noexcept {
   auto* widget = this->ui.targets;
   auto* model  = (QStandardItemModelDKEx*)widget->model();
   auto  qmi    = model->index(index, 0, QModelIndex());
   widget->setCurrentIndex(qmi);
}

int QuestTabObjectives::_selected_objective_id() const noexcept {
   auto* widget = this->ui.objectives;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return no_stage;
   auto index = sm->currentIndex();
   auto data  = widget->model()->data(index, Qt::UserRole);
   if (!data.isValid())
      return no_stage;
   return data.toInt();
}
int QuestTabObjectives::_selected_target_index() const noexcept {
   auto* widget = this->ui.targets;
   auto* sm     = widget->selectionModel();
   if (!sm)
      return -1;
   auto index = sm->currentIndex();
   if (!index.isValid())
      return -1;
   return index.row();
}

void QuestTabObjectives::_redraw_objective_list() {
   int   prior_id = this->_selected_objective_id(); // prior selected stage ID
   auto* widget   = this->ui.objectives;
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
void QuestTabObjectives::_redraw_objective_settings() {
   auto* ptr = this->_get_objective();
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
void QuestTabObjectives::_redraw_target_list() {
   int   index   = this->_selected_target_index();
   auto* widget  = this->ui.targets;
   auto  blocker = QSignalBlocker(widget);
   auto* model   = (QStandardItemModelDKEx*) widget->model();
   assert(model);
   model->clearBody();
   //
   auto* ptr = this->_get_objective();
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
void QuestTabObjectives::_redraw_target_settings() {
   auto* ptr = this->_get_target();
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

void QuestTabObjectives::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   if (this->_did_first_show)
      return;
   this->_did_first_show = true;
   if (auto* header = this->ui.targets->horizontalHeader()) {
      auto width = header->width();
      auto third = width / 3;
      header->resizeSection(0, width - third);
      header->resizeSection(1, 0); // set the last section to minimum size and let it stretch; that way, enlarging the prior sections doesn't cause this one to clip out of bounds
   }
}