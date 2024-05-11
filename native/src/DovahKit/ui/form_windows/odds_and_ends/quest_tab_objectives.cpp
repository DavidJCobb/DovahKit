#include "quest_tab_objectives.h"
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include "helpers/qt/basic_bindings.h"
#include "../../generic/QStandardItemModelDKEx.h" // enhanced QStandardItemModel
#include "dovah/forms/components/conditions/context.h"
#include "dovah/core.h"
#include "editor/core.h"
#include "editor/helpers/stringify_conditions.h"

namespace {
   constexpr int no_stage = -1;
   static_assert(std::numeric_limits<decltype(dovah::loaded_forms::Quest::Stage::index)>::min() > no_stage, "The sentinel value that this UI uses for \"no stage\" needs to be outside of the range of valid stage IDs.");
}

QuestTabObjectives::QuestTabObjectives(dovah::form_stub& s, loaded_t& q, QWidget* parent) : QWidget(parent), stub(s), form(q) {
   ui.setupUi(this);

   #pragma region Objective list
      {
         auto* widget = this->ui.objectives;
         auto* model  = new QStandardItemModelDKEx(widget);
         model->setAutoTooltips(true); // QStandardItemModelDKEx
         model->setColumnCount(2);
         model->setSortRole(Qt::UserRole);
         widget->setModel(model);
         model->setHorizontalHeaderLabels({ tr("Index"), tr("Text") });
         widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
         widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
         widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         //
         auto* header = widget->horizontalHeader();
         header->setDefaultAlignment(Qt::AlignLeft);
         header->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
         header->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
         header->setSortIndicatorShown(false);
         header->setStretchLastSection(true);
         model->setHeaderData(0, Qt::Horizontal, (int)(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
         {
            auto* vh = widget->verticalHeader();
            vh->setVisible(false);
            vh->setDefaultSectionSize(vh->minimumSectionSize());
         }
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
         auto& obj = this->form.objectives.emplace_back();
         obj.index = value;
         this->_redraw_objective_list();
         this->_select_objective(value);
      });
      QObject::connect(this->context_menu_actions.objective_list.remove, &QAction::triggered, this, [this]() {
         int prev = std::numeric_limits<int>::min();
         int next = std::numeric_limits<int>::max();
         int sel  = this->_selected_objective_id();
         if (sel < 0) // exit if no selection
            return;
         for (auto& obj : this->form.objectives) {
            auto id = obj.index;
            if (id < sel) {
               if (id > prev)
                  prev = id;
            } else if (id > sel) {
               if (id < next)
                  next = id;
            }
         }
         this->form.remove_objective(sel);
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
   
   #pragma region Objective settings
   QObject::connect(this->ui.objectiveID, &QSpinBox::editingFinished, this, [this]() {
      auto* widget = this->ui.objectiveID;
      if (auto* obj = this->_get_objective()) {
         int   value = widget->value();
         auto* other = this->_get_objective(value);
         if (other && other != obj) {
            //
            // Using a QValidator doesn't work. Why? Well, suppose we try to change Objective 40 
            // into Objective 30 when we already have an Objective 30. What we want to happen is, 
            // no change is made and we revert back to 40. Here's the problem, however: how do we 
            // actually change it to Stage 30? Well, we clear the value, we type a 3, and then we 
            // type an 0.
            //
            // If we use a QValidator to reject the change to 30, then the value changes to just 
            // 3, since that was the last valid value entered. Not intuitive behavior.
            //
            QApplication::beep();
            const auto blocker = QSignalBlocker(widget);
            widget->setValue(obj->index);
            return;
         }
         obj->index = value;
         this->_redraw_objective_list();
         this->_select_objective(value);
      }
   });
   //
   QObject::connect(this->ui.objectiveFlagOR, &QCheckBox::stateChanged, this, [this](int state) {
      if (auto* obj = this->_get_objective())
         cobb::edit_bit(obj->flags, loaded_t::Objective::flag::or_with_previous, state == Qt::CheckState::Checked);
   });
   QObject::connect(this->ui.objectiveText, &QLineEdit::textEdited, this, [this](const QString& text) {
      if (auto* obj = this->_get_objective()) {
         DovahKitCore::get().assign_localized_string(obj->text, text);
         this->redrawObjectiveListSelectedItemText();
      }
   });
   #pragma endregion
   
   #pragma region Target list
      {
         auto* widget = this->ui.targets;
         auto* model  = new QStandardItemModelDKEx(widget);
         model->setAutoTooltips(true); // QStandardItemModelDKEx
         model->setColumnCount(2);
         widget->setModel(model);
         model->setHorizontalHeaderLabels({ tr("Alias"), tr("Conditions") });
         widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
         widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
         widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
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
         QObject::connect(this->ui.targetConditions, &DKConditionList::changeAttempted, this, [this]() {
            if (auto* t = this->_get_target()) {
               this->ui.targetConditions->exportTo(this->form, t->conditions);
            }
            this->redrawTargetListSelectedItemConditions();
         });
      }
      #pragma region Context menu
      this->context_menu_actions.target_list.insert   = new QAction(tr("New..."),    this->ui.targets);
      this->context_menu_actions.target_list.remove   = new QAction(tr("Delete"),    this->ui.targets);
      this->context_menu_actions.target_list.moveUp   = new QAction(tr("Move up"),   this->ui.targets);
      this->context_menu_actions.target_list.moveDown = new QAction(tr("Move down"), this->ui.targets);
      //
      QObject::connect(this->context_menu_actions.target_list.insert, &QAction::triggered, this, [this]() {
         auto* obj = this->_get_objective();
         if (!obj)
            return;
         auto index = obj->targets.size();
         obj->targets.emplace_back();
         this->_redraw_target_list();
         this->_select_target(index);
      });
      QObject::connect(this->context_menu_actions.target_list.remove, &QAction::triggered, this, [this]() {
         auto* obj = this->_get_objective();
         if (!obj)
            return;
         auto  ti   = this->_selected_target_index();
         auto& list = obj->targets;
         if (ti < 0 || ti >= list.size())
            return;
         list[ti].clear(this->form);
         list.erase(list.begin() + ti);
         this->_redraw_target_list();
         //
         if (ti < obj->targets.size())
            this->_select_target(ti);
         else if (ti > 0)
            this->_select_target(ti - 1);
      });
      QObject::connect(this->context_menu_actions.target_list.moveUp, &QAction::triggered, this, [this]() {
         auto* obj = this->_get_objective();
         if (!obj)
            return;
         auto& list = obj->targets;
         int   e    = this->_selected_target_index();
         if (e <= 0)
            return;
         std::swap(list[e], list[e - 1]);
         this->_redraw_target_list();
         this->_select_target(e - 1);
      });
      QObject::connect(this->context_menu_actions.target_list.moveDown, &QAction::triggered, this, [this]() {
         auto* obj = this->_get_objective();
         if (!obj)
            return;
         auto& list = obj->targets;
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
            int   row = rows[0].row();
            auto* obj = this->_get_objective();
            this->context_menu_actions.target_list.moveUp->setEnabled(row > 0);
            this->context_menu_actions.target_list.moveDown->setEnabled(obj && row + 1 < obj->targets.size());
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

   #pragma region Targets
      #pragma region Settings
      QObject::connect(this->ui.targetAlias, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
         if (auto* target = this->_get_target()) {
            auto data = this->ui.targetAlias->currentData();
            target->aliasID = data.isValid() ? data.toInt() : -1;
            this->redrawTargetListSelectedItemAliasName();
         }
      });
      QObject::connect(this->ui.targetFlagIgnoreLocks, &QCheckBox::stateChanged, this, [this](int state) {
         if (auto* entry = this->_get_target())
            cobb::edit_bit(entry->flags, loaded_t::Target::flag::marker_pathing_ignores_locks, state == Qt::CheckState::Checked);
      });
      #pragma endregion
   #pragma endregion

   this->_redraw_alias_picker();
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
   QObject::disconnect(this->ui.objectives);
   QObject::disconnect(this->ui.objectiveFlagOR);
   QObject::disconnect(this->ui.objectiveID);
   QObject::disconnect(this->ui.objectiveText);
   QObject::disconnect(this->ui.targets);
   QObject::disconnect(this->ui.targetAlias);
   QObject::disconnect(this->ui.targetFlagIgnoreLocks);
   this->ui.targetConditions->clear();
}

void QuestTabObjectives::redrawObjectiveListSelectedItemText() {
   loaded_t::Objective* obj = this->_get_objective();
   if (!obj)
      return;
   //
   auto* widget = this->ui.objectives;
   auto  qmi    = widget->currentIndex();
   if (!qmi.isValid())
      return;
   auto* model  = (QStandardItemModelDKEx*) widget->model();
   auto* col    = model->item(qmi.row(), 1);
   if (!col)
      return;
   col->setText(DovahKitCore::get().convert_localized_string(obj->text));
}
void QuestTabObjectives::redrawTargetListSelectedItemAliasName() {
   loaded_t::Target* target = nullptr;
   //
   auto* widget = this->ui.targets;
   auto  index  = widget->currentIndex();
   if (!index.isValid())
      return;
   auto row = index.row();
   //
   if (auto* s = this->_get_objective()) {
      auto& list = s->targets;
      if (row < 0 || row >= list.size())
         return;
      target = &list[row];
   }
   if (!target)
      return;
   //
   auto* model = (QStandardItemModelDKEx*)widget->model();
   auto* col0  = model->item(row, 0);
   if (!col0)
      return;
   col0->setText(this->_get_alias_name(target->aliasID));
}
void QuestTabObjectives::redrawTargetListSelectedItemConditions() {
   loaded_t::Target* target = nullptr;
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
   if (auto* s = this->_get_objective()) {
      auto& list = s->targets;
      if (row < 0 || row >= list.size())
         return;
      target = &list[row];
   }
   if (!target)
      return;
   //
   auto* col1 = model->item(row, 1);
   if (!col1)
      return;
   auto  ctx  = dovah::loaded_forms::components::conditions::context(this->stub, true);
   col1->setText(editor_helpers::stringify_condition_list(target->conditions, ctx));
}

dovah::loaded_forms::Alias* QuestTabObjectives::_get_alias(int id) const noexcept {
   return this->form.lookup_alias_by_id(id);
}
QString QuestTabObjectives::_get_alias_name(int id) const noexcept {
   if (auto* alias = this->_get_alias(id))
      return QString::fromStdString(alias->name);
   return tr("NONE", "alias name in quest objective tab");
}

QuestTabObjectives::loaded_t::Objective* QuestTabObjectives::_get_objective() const noexcept {
   return this->_get_objective(this->_selected_objective_id());
}
QuestTabObjectives::loaded_t::Objective* QuestTabObjectives::_get_objective(int id) const noexcept {
   if (id < 0)
      return nullptr;
   for (auto& s : this->form.objectives)
      if (s.index == id)
         return &s;
   return nullptr;
}
QuestTabObjectives::loaded_t::Target* QuestTabObjectives::_get_target() const noexcept {
   if (auto* s = this->_get_objective()) {
      auto  i    = this->_selected_target_index();
      auto& list = s->targets;
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
   auto& list = s->targets;
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

void QuestTabObjectives::_redraw_alias_picker() {
   int prior_id = -1;
   if (auto* target = this->_get_target())
      prior_id = target->aliasID;
   //
   auto* widget  = this->ui.targetAlias;
   auto  blocker = QSignalBlocker(widget);
   //
   widget->clear();
   this->form.for_each_alias_of_type(dovah::loaded_forms::Alias::alias_type::reference, [widget](dovah::loaded_forms::Alias* alias) {
      if (widget->findData(alias->id) >= 0)
         return false; // continue
      widget->addItem(QString::fromStdString(alias->name), alias->id);
      return false; // continue
   });
   widget->model()->sort(0);
   widget->insertItem(0, tr("NONE", "alias name in quest objective tab"), -1);
   //
   auto i = widget->findData(prior_id);
   if (i < 0)
      i = 0;
   widget->setCurrentIndex(i);
}
void QuestTabObjectives::_redraw_objective_list() {
   int   prior_id = this->_selected_objective_id();
   auto* widget   = this->ui.objectives;
   auto  blocker  = QSignalBlocker(widget);
   auto* model    = (QStandardItemModelDKEx*) widget->model();
   assert(model);
   model->clearBody();
   //
   auto& list = this->form.objectives;
   if (list.empty())
      return;
   QStandardItem* prior = nullptr;
   auto& editor = DovahKitCore::get();
   for (auto& obj : list) {
      auto* col0 = new QStandardItem(QString::number(obj.index));
      auto* col1 = new QStandardItem(editor.convert_localized_string(obj.text));
      col0->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
      col0->setData((int)obj.index, Qt::UserRole);
      col1->setData((int)obj.index, Qt::UserRole); // *sigh* having absolutely everything be per-cell and not per-row is annoying
      model->appendRow({ col0, col1 });
      //
      if (obj.index == prior_id)
         prior = col0;
   }
   model->sort(0);
   //
   if (prior) {
      widget->setCurrentIndex(prior->index());
   }
}
void QuestTabObjectives::_redraw_objective_settings() {
   auto* ptr = this->_get_objective();
   this->ui.objectiveFlagOR->setEnabled(ptr != nullptr);
   this->ui.objectiveID->setEnabled(ptr != nullptr);
   this->ui.objectiveText->setEnabled(ptr != nullptr);
   this->ui.targets->setEnabled(ptr != nullptr);
   //
   const auto blocker0 = QSignalBlocker(this->ui.objectiveFlagOR);
   const auto blocker1 = QSignalBlocker(this->ui.objectiveID);
   const auto blocker2 = QSignalBlocker(this->ui.objectiveText);
   if (!ptr) {
      this->ui.objectiveFlagOR->setChecked(false);
      this->ui.objectiveID->clear();
      this->ui.objectiveText->clear();
      return;
   }
   this->ui.objectiveFlagOR->setChecked(ptr->flags & loaded_t::Objective::flag::or_with_previous);
   this->ui.objectiveID->setValue(ptr->index);
   this->ui.objectiveText->setText(DovahKitCore::get().convert_localized_string(ptr->text));
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
   auto  ctx  = dovah::loaded_forms::components::conditions::context(this->stub, true);
   auto& list = ptr->targets;
   auto  size = list.size();
   QModelIndex prior;
   for (size_t i = 0; i < size; ++i) {
      auto& target = list[i];
      auto* col0   = new QStandardItem(this->_get_alias_name(target.aliasID));
      auto* col1   = new QStandardItem(editor_helpers::stringify_condition_list(target.conditions, ctx));
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
   this->ui.targetFlagIgnoreLocks->setEnabled(ptr != nullptr);
   this->ui.targetAlias->setEnabled(ptr != nullptr);
   this->ui.targetConditions->setEnabled(ptr != nullptr);
   //
   const auto blocker0 = QSignalBlocker(this->ui.targetFlagIgnoreLocks);
   const auto blocker1 = QSignalBlocker(this->ui.targetAlias);
   //
   if (!ptr) {
      this->ui.targetFlagIgnoreLocks->setChecked(false);
      this->ui.targetAlias->setCurrentIndex(this->ui.targetAlias->findData(-1));
      this->ui.targetConditions->clear();
      return;
   }
   //
   this->ui.targetFlagIgnoreLocks->setChecked(ptr->flags & loaded_t::Target::flag::marker_pathing_ignores_locks);
   this->ui.targetAlias->setCurrentIndex(this->ui.targetAlias->findData(ptr->aliasID));
   this->ui.targetConditions->importFrom(this->form, ptr->conditions);
}

void QuestTabObjectives::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   //
   this->_redraw_alias_picker(); // aliases could've been changed in another tab
   this->_redraw_target_list();  // aliases could've been changed in another tab
   //
   if (this->_did_first_show)
      return;
   this->_did_first_show = true;
   if (auto* header = this->ui.objectives->horizontalHeader()) {
      auto metrics = QFontMetrics(this->ui.targets->font());
      auto index   = metrics.boundingRect("655350").width() * 1.5F + 4;
      header->resizeSection(0, index);
      header->resizeSection(1, 0); // set the last section to minimum size and let it stretch; that way, enlarging the prior sections doesn't cause this one to clip out of bounds
   }
   if (auto* header = this->ui.targets->horizontalHeader()) {
      auto width = header->width();
      auto third = width / 3;
      header->resizeSection(0, third);
      header->resizeSection(1, 0); // set the last section to minimum size and let it stretch; that way, enlarging the prior sections doesn't cause this one to clip out of bounds
   }
}