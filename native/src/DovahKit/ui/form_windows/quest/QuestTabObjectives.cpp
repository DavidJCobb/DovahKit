#include "./QuestTabObjectives.h"
#include "helpers/arrays/construct_from.h"
#pragma region Widget includes
   #include <QAction>
   #include <QCheckBox>
   #include <QComboBox>
   #include <QLineEdit>
   #include <QSpinBox>
   #include <QTableView>
   #include "widgets/DKConditionList.h"
#pragma endregion
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "./QuestAliasesModel.h"
#include "./QuestObjectivesModel.h"
#include "./QuestObjectiveTargetsModel.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/size_tableview_columns.h"
#include "ui/utils/typical_tableview_config.h"

QuestTabObjectives::QuestTabObjectives(quest_form_type& quest, QWidget* parent) : QObject(parent), working_quest(quest) {
   this->models.objectives = new QuestObjectivesModel(this);
   this->models.targets    = new QuestObjectiveTargetsModel(this);
   this->models.targets->setSourceModel(this->models.objectives);
}
QuestTabObjectives::~QuestTabObjectives() {
}
void QuestTabObjectives::setupUi() {
   {  // Objectives
      auto* model   = this->models.objectives;
      auto* view    = this->ui.objectives.view;
      auto& context = this->context.objectives;

      view->setModel(model);
      view->setWordWrap(false);
      ui::set_custom_context_menu(*view, context.menu);
      ui::typical_tableview_config(view);
      ui::size_tableview_columns<std::array<ui::tableview_column_spec, QuestObjectivesModel::ColumnCount>{
         ui::tableview_column_spec{ // Objective Index
            .grow   = 0,
            .shrink = 0,
         },
         ui::tableview_column_spec{ // OR
            .grow   = 0,
            .shrink = 0,
         },
         ui::tableview_column_spec{ // Display Text
            .grow   = 2,
            .shrink = 0,
         },
      }>(view);

      {  // Context menu
         auto& menu    = context.menu;
         auto& actions = context.actions;
         {
            auto* action = actions.create = new QAction(tr("New..."));
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, [view, model]() {
               auto qmi = model->createObjective();
               if (!qmi.isValid())
                  return;
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(QuestObjectivesModel::ColumnCount - 1);
               view->selectionModel()->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            });
         }
         {
            auto* action = actions.remove = new QAction(tr("Delete"));
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, [view, model]() {
               auto rows = view->selectionModel()->selectedRows();
               if (rows.empty())
                  return;
               model->deleteObjective(rows[0]);
            });
         }
         QObject::connect(&menu, &QMenu::aboutToShow, this, [this, &context]() {
            context.actions.remove->setEnabled(this->_selected_objective_qmi().isValid());
         });
      }

      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QuestTabObjectives::_on_objective_selected);

      // Loading data into the model fires `modelReset`, which means we're basically pretending to delete 
      // the no-objective QMI, which means the targets tableview ceases to use that as its root index. If 
      // we call `_on_objective_selected`, it should get that fixed right up.
      QObject::connect(model, &QAbstractItemModel::modelReset, this, &QuestTabObjectives::_on_objective_selected);

      QObject::connect(this->ui.objectives.current.flag_or, &QCheckBox::toggled, this, &QuestTabObjectives::_on_objective_data_edited);
      QObject::connect(this->ui.objectives.current.index,   qOverload<int>(&QSpinBox::valueChanged), this, &QuestTabObjectives::_on_objective_data_edited);
      QObject::connect(this->ui.objectives.current.text,    &QLineEdit::textEdited, this, &QuestTabObjectives::_on_objective_data_edited);
   }
   {  // Targets
      auto* model   = this->models.targets;
      auto* view    = this->ui.targets.view;
      auto& context = this->context.targets;

      view->setModel(model);
      view->setWordWrap(false);
      ui::set_custom_context_menu(*view, context.menu);
      ui::typical_tableview_config(view);
      ui::size_tableview_columns<std::array<ui::tableview_column_spec, QuestObjectiveTargetsModel::ColumnCount>{
         ui::tableview_column_spec{ // Target Ref
            .grow   = 1,
            .shrink = 1,
         },
         ui::tableview_column_spec{ // Conditions
            .grow   = 3,
            .shrink = 1,
         },
      }>(view);

      {  // Context menu
         auto& menu    = context.menu;
         auto& actions = context.actions;
         {
            auto* action = actions.create = new QAction(tr("New..."));
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, [view, model]() {
               auto qmi = model->createTarget(view->rootIndex());
               if (!qmi.isValid())
                  return;
               auto tl = qmi.siblingAtColumn(0);
               auto br = qmi.siblingAtColumn(QuestObjectiveTargetsModel::ColumnCount - 1);
               view->selectionModel()->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            });
         }
         {
            auto* action = actions.remove = new QAction(tr("Delete"));
            menu.addAction(action);
            QObject::connect(action, &QAction::triggered, this, [view, model]() {
               auto rows = view->selectionModel()->selectedRows();
               if (rows.empty())
                  return;
               model->deleteTarget(rows[0]);
            });
         }
         QObject::connect(&menu, &QMenu::aboutToShow, this, [this, &context]() {
            context.actions.create->setEnabled(this->_selected_objective_qmi().isValid());
            context.actions.remove->setEnabled(this->_selected_target_qmi().isValid());
         });
      }

      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QuestTabObjectives::_on_target_selected);

      this->_populate_alias_picker();

      QObject::connect(this->ui.targets.current.alias,        qOverload<int>(&QComboBox::currentIndexChanged), this, &QuestTabObjectives::_on_target_data_edited);
      QObject::connect(this->ui.targets.current.ignore_locks, &QCheckBox::toggled, this, &QuestTabObjectives::_on_target_data_edited);
      QObject::connect(this->ui.targets.current.conditions,   &DKConditionList::changed, this, [this]() {
         auto qmi = this->_selected_target_qmi();
         if (!qmi.isValid())
            return;
         QuestObjectiveTargetsModel::condition_list list;
         this->ui.targets.current.conditions->exportTo(this->working_quest, list);
         this->models.targets->setTargetConditions(qmi, std::move(list));
      });
   }

   this->_on_objective_selected();
}
void QuestTabObjectives::setAliasesModel(const QuestAliasesModel* model) {
   this->models.objectives->setAliasesModel(model);
   if (this->aliases_model) {
      QObject::disconnect(this->aliases_model, nullptr, this, nullptr);
   }
   this->aliases_model = model;
   if (model) {
      QObject::connect(model, &QuestAliasesModel::dataChanged, this, [this](const QModelIndex& tl, const QModelIndex& br) {
         if (tl.column() <= QuestAliasesModel::Column::Name && QuestAliasesModel::Column::Name <= br.column()) {
            auto  alias_id = tl.siblingAtColumn(QuestAliasesModel::Column::ID).data(Qt::EditRole).toInt();
            auto* picker   = this->ui.targets.current.alias;
            auto  i        = picker->findData(alias_id);
            if (i >= 0) {
               picker->setItemText(i, tl.siblingAtColumn(QuestAliasesModel::Column::Name).data(Qt::EditRole).toString());
               this->_re_sort_alias_picker();
            }
         }
      });
      QObject::connect(model, &QuestAliasesModel::rowsInserted, this, [this](const QModelIndex&, int first, int last) {
         auto* picker = this->ui.targets.current.alias;
         for (auto i = first; i <= last; ++i) {
            auto  qmi   = this->aliases_model->index(i, 0, {});
            auto* alias = this->aliases_model->alias(qmi);
            if (!alias)
               continue;
            if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
               continue;
            auto name = QString::fromStdString(alias->name);
            picker->addItem(name, alias->id);
         }
         this->_re_sort_alias_picker();
      });
      QObject::connect(model, &QuestAliasesModel::rowsRemoved, this, &QuestTabObjectives::_populate_alias_picker);
      QObject::connect(model, &QuestAliasesModel::modelReset, this, &QuestTabObjectives::_populate_alias_picker);
      this->_populate_alias_picker();
   }
}

void QuestTabObjectives::load() {
   this->models.objectives->load(this->working_quest);
}
void QuestTabObjectives::save() {
   this->models.objectives->save(this->working_quest);
}

void QuestTabObjectives::_populate_alias_picker() {
   auto* picker = this->ui.targets.current.alias;
   auto  prior  = picker->currentData().toInt();
   if (picker->currentIndex() == -1)
      prior = -1;
   {
      const auto blocker = QSignalBlocker(picker);
      picker->clear();
      if (this->aliases_model) {
         size_t count = this->aliases_model->rowCount({});
         for (size_t i = 0; i < count; ++i) {
            auto  qmi   = this->aliases_model->index(i, 0, {});
            auto* alias = this->aliases_model->alias(qmi);
            if (!alias)
               continue;
            if (alias->type != dovah::loaded_forms::Alias::alias_type::reference)
               continue;
            auto name = QString::fromStdString(alias->name);
            picker->addItem(name, alias->id);
         }
         picker->model()->sort(0);
      }
      picker->insertItem(0, tr("NONE"), -1);
   }
   if (prior != picker->currentData().toInt()) {
      emit picker->currentIndexChanged(picker->currentIndex());
   }
}
void QuestTabObjectives::_re_sort_alias_picker() {
   auto* picker = this->ui.targets.current.alias;
   auto  prior  = picker->currentData().toInt();
   const auto blocker = QSignalBlocker(picker);
   {
      auto i = picker->findData(-1);
      if (i >= 0)
         picker->removeItem(i);
   }
   picker->model()->sort(0);
   picker->insertItem(0, tr("NONE"), -1);
   if (prior == -1)
      picker->setCurrentIndex(0);
}

QModelIndex QuestTabObjectives::_selected_objective_qmi() {
   auto rows = this->ui.objectives.view->selectionModel()->selectedRows();
   if (!rows.empty())
      return rows[0];
   return {};
}
void QuestTabObjectives::_on_objective_selected() {
   using model_type = QuestObjectivesModel;

   const auto widgets = std::array<QWidget*, 4>{
      this->ui.objectives.current.flag_or,
      this->ui.objectives.current.index,
      this->ui.objectives.current.text,
      this->ui.targets.view,
   };
   const auto blockers = cobb::arrays::construct_from<QSignalBlocker, QWidget*>(widgets);

   auto qmi = _selected_objective_qmi();
   if (!qmi.isValid()) {
      for (auto* widget : widgets)
         widget->setEnabled(false);
      this->ui.objectives.current.flag_or->setChecked(false);
      this->ui.objectives.current.index->setValue(0);
      this->ui.objectives.current.text->setText({});
      this->ui.targets.view->setRootIndex(this->models.targets->mapFromSource(this->models.objectives->noObjectiveQMI()));
      this->ui.targets.view->selectionModel()->select(QModelIndex{}, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      this->_on_target_selected();
      return;
   }
   for (auto* widget : widgets)
      widget->setEnabled(true);

   this->ui.objectives.current.flag_or->setChecked(qmi.data(model_type::IsOrRole).toBool());
   this->ui.objectives.current.index->setValue(qmi.data(model_type::IndexRole).toInt());
   this->ui.objectives.current.text->setText(qmi.data(model_type::TextRole).toString());

   this->ui.targets.view->setRootIndex(this->models.targets->mapFromSource(qmi));
   this->ui.targets.view->selectionModel()->select(QModelIndex{}, QItemSelectionModel::SelectionFlag::ClearAndSelect);
   this->_on_target_selected();
}
void QuestTabObjectives::_on_objective_data_edited() {
   using model_type = QuestObjectivesModel;

   auto qmi = _selected_objective_qmi();
   if (!qmi.isValid())
      return;

   auto* model = this->models.objectives;
   model->setData(qmi, this->ui.objectives.current.flag_or->isChecked(), model_type::IsOrRole);
   model->setData(qmi, this->ui.objectives.current.index->value(), model_type::IndexRole);
   model->setData(qmi, this->ui.objectives.current.text->text(), model_type::TextRole);
}

QModelIndex QuestTabObjectives::_selected_target_qmi() {
   auto rows = this->ui.targets.view->selectionModel()->selectedRows();
   if (!rows.empty())
      return rows[0];
   return {};
}
void QuestTabObjectives::_on_target_selected() {
   using model_type = QuestObjectiveTargetsModel;

   auto* alias_picker = this->ui.targets.current.alias;

   const auto widgets = std::array<QWidget*, 3>{
      alias_picker,
      this->ui.targets.current.ignore_locks,
      this->ui.targets.current.conditions
   };
   const auto blockers = cobb::arrays::construct_from<QSignalBlocker>(widgets);

   auto qmi = _selected_target_qmi();
   if (!qmi.isValid()) {
      for (auto* widget : widgets)
         widget->setEnabled(false);
      alias_picker->setCurrentIndex(alias_picker->findData(-1));
      this->ui.targets.current.ignore_locks->setChecked(false);
      this->ui.targets.current.conditions->clear();
      return;
   }
   for (auto* widget : widgets)
      widget->setEnabled(true);
   alias_picker->setCurrentIndex(alias_picker->findData(qmi.data(model_type::AliasIDRole).toInt()));
   this->ui.targets.current.ignore_locks->setChecked(qmi.data(model_type::IgnoreLocksRole).toBool());
   {
      auto* widget = this->ui.targets.current.conditions;
      widget->clear();
      widget->importFrom(this->working_quest, this->models.targets->targetConditions(qmi));
   }
}
void QuestTabObjectives::_on_target_data_edited() {
   using model_type = QuestObjectiveTargetsModel;

   auto qmi = _selected_target_qmi();
   if (!qmi.isValid())
      return;

   auto* model = this->models.targets;
   model->setData(qmi, this->ui.targets.current.alias->currentData().toInt(), model_type::AliasIDRole);
   model->setData(qmi, this->ui.targets.current.ignore_locks->isChecked(), model_type::IgnoreLocksRole);
   //
   // Conditions have their own signal handler, to avoid unnecessary list copying
}