#include "./QuestTabStages.h"
#include "helpers/arrays/construct_from.h"
#pragma region Widget includes
   #include <QCheckBox>
   #include <QListView>
   #include <QPlainTextEdit>
   #include <QPushButton>
   #include <QSpinBox>
   #include <QTableView>
   #include "widgets/DKConditionList.h"
   #include "widgets/DKFormPicker.h"
   #include "widgets/DKPapyrusFragmentFunctionPicker.h"
#pragma endregion
#include "dovah/forms/Quest.h"
#include "editor/core.h"
#include "./QuestStagesModel.h"
#include "./QuestStageLogEntriesModel.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/size_tableview_columns.h"
#include "ui/utils/typical_tableview_config.h"

QuestTabStages::QuestTabStages(quest_form_type& quest, QWidget* parent) : QObject(parent), working_quest(quest) {
   this->models.stages      = new QuestStagesModel(this);
   this->models.log_entries = new QuestStageLogEntriesModel(this);
   this->models.log_entries->setSourceModel(this->models.stages);
}
QuestTabStages::~QuestTabStages() {
}
void QuestTabStages::setupUi() {
   {  // Stages
      auto* model   = this->models.stages;
      auto* view    = this->ui.stages.view;

      view->setModel(model);
      view->setWordWrap(false);

      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QuestTabStages::_on_stage_selected);

      QObject::connect(this->ui.stages.buttons.create, &QPushButton::clicked, this, [view, model]() {
         auto qmi = model->createStage();
         if (!qmi.isValid())
            return;
         view->selectionModel()->select({ qmi, qmi }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      });
      QObject::connect(this->ui.stages.buttons.remove, &QPushButton::clicked, this, [this, model]() {
         auto qmi = this->_selected_stage_qmi();
         if (qmi.isValid())
            model->deleteStage(qmi);
      });

      QObject::connect(this->ui.stages.current.keep_instance_data, &QCheckBox::toggled, this, &QuestTabStages::_on_stage_data_edited);
      QObject::connect(this->ui.stages.current.id,       qOverload<int>(&QSpinBox::valueChanged), this, &QuestTabStages::_on_stage_data_edited);
      QObject::connect(this->ui.stages.current.shutdown, &QCheckBox::toggled, this, &QuestTabStages::_on_stage_data_edited);
      QObject::connect(this->ui.stages.current.startup,  &QCheckBox::toggled, this, &QuestTabStages::_on_stage_data_edited);
   }
   {  // Log Entries
      auto* model   = this->models.log_entries;
      auto* view    = this->ui.log_entries.view;

      view->setModel(model);
      view->setWordWrap(false);
      ui::typical_tableview_config(view);
      ui::size_tableview_columns<std::array<ui::tableview_column_spec, QuestStageLogEntriesModel::ColumnCount>{
         ui::tableview_column_spec{ // Log Entry Text
            .grow   = 1,
            .shrink = 1,
         },
         ui::tableview_column_spec{ // Conditions
            .grow   = 3,
            .shrink = 1,
         },
      }>(view);
      
      QObject::connect(this->ui.log_entries.buttons.create, &QPushButton::clicked, this, [this, view, model]() {
         auto qmi = model->createLogEntry(this->_selected_stage_qmi());
         if (!qmi.isValid())
            return;
         auto tl = qmi.siblingAtColumn(0);
         auto br = qmi.siblingAtColumn(QuestStageLogEntriesModel::ColumnCount - 1);
         view->selectionModel()->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      });
      QObject::connect(this->ui.log_entries.buttons.remove, &QPushButton::clicked, this, [this, model]() {
         auto qmi = this->_selected_stage_qmi();
         if (qmi.isValid())
            model->deleteLogEntry(qmi);
      });

      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QuestTabStages::_on_log_entry_selected);

      // Loading data into the model fires `modelReset`, which means we're basically pretending to delete 
      // the no-stage QMI, which means the log entries tableview ceases to use that as its root index. If 
      // we call `_on_stage_selected`, it should get that fixed right up.
      QObject::connect(model, &QAbstractItemModel::modelReset, this, &QuestTabStages::_on_stage_selected);

      QObject::connect(this->ui.log_entries.current.complete,    &QCheckBox::toggled, this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.conditions,  &DKConditionList::changed, this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.fail,        &QCheckBox::toggled, this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.fragment,    &DKPapyrusFragmentFunctionPicker::currentScriptnameChanged, this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.fragment,    &DKPapyrusFragmentFunctionPicker::currentFunctionChanged,   this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.next_quest,  &DKFormPicker::formChanged, this, &QuestTabStages::_on_log_entry_data_edited);
      QObject::connect(this->ui.log_entries.current.text,        &QPlainTextEdit::textChanged, this, &QuestTabStages::_on_log_entry_data_edited);
   }

   this->_on_stage_selected();
}

void QuestTabStages::load() {
   this->models.stages->load(this->working_quest);
}
void QuestTabStages::save() {
   this->models.stages->save(this->working_quest);
}

QModelIndex QuestTabStages::_selected_stage_qmi() {
   auto rows = this->ui.stages.view->selectionModel()->selectedRows();
   if (!rows.empty())
      return rows[0];
   return {};
}
void QuestTabStages::_on_stage_selected() {
   using model_type = QuestStagesModel;

   const auto widgets = std::array<QWidget*, 5>{
      this->ui.stages.current.id,
      this->ui.stages.current.keep_instance_data,
      this->ui.stages.current.shutdown,
      this->ui.stages.current.startup,
      this->ui.log_entries.view,
   };
   const auto blockers = cobb::arrays::construct_from<QSignalBlocker, QWidget*>(widgets);

   auto        qmi  = _selected_stage_qmi();
   const auto* data = this->models.stages->stage(qmi);
   if (!data || !qmi.isValid()) {
      for (auto* widget : widgets)
         widget->setEnabled(false);
      this->ui.stages.current.id->setValue(0);
      this->ui.stages.current.keep_instance_data->setChecked(false);
      this->ui.stages.current.shutdown->setChecked(false);
      this->ui.stages.current.startup->setChecked(false);
      this->ui.log_entries.view->setRootIndex(this->models.log_entries->mapFromSource(this->models.stages->noStageQMI()));
      this->ui.log_entries.view->selectionModel()->select(QModelIndex{}, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      this->_on_log_entry_selected();
      return;
   }
   for (auto* widget : widgets)
      widget->setEnabled(true);

   this->ui.stages.current.id->setValue(data->id);
   this->ui.stages.current.keep_instance_data->setChecked(data->keep_instance_data);
   this->ui.stages.current.shutdown->setChecked(data->shutdown);
   this->ui.stages.current.startup->setChecked(data->startup);

   this->ui.log_entries.view->setRootIndex(this->models.log_entries->mapFromSource(qmi));
   this->ui.log_entries.view->selectionModel()->select(QModelIndex{}, QItemSelectionModel::SelectionFlag::ClearAndSelect);
   this->_on_log_entry_selected();
}
void QuestTabStages::_on_stage_data_edited() {
   using model_type = QuestStagesModel;
   
   auto qmi = _selected_stage_qmi();
   if (!qmi.isValid())
      return;

   model_type::StageData data;
   data.id = this->ui.stages.current.id->value();
   data.keep_instance_data = this->ui.stages.current.keep_instance_data->isChecked();
   data.shutdown = this->ui.stages.current.shutdown->isChecked();
   data.startup = this->ui.stages.current.startup->isChecked();

   this->models.stages->setStage(qmi, data);
}

QModelIndex QuestTabStages::_selected_log_entry_qmi() {
   auto rows = this->ui.log_entries.view->selectionModel()->selectedRows();
   if (!rows.empty())
      return rows[0];
   return {};
}
void QuestTabStages::_on_log_entry_selected() {
   using model_type = QuestStageLogEntriesModel;

   const auto widgets = std::array<QWidget*, 6>{
      this->ui.log_entries.current.complete,
      this->ui.log_entries.current.conditions,
      this->ui.log_entries.current.fail,
      this->ui.log_entries.current.fragment,
      this->ui.log_entries.current.next_quest,
      this->ui.log_entries.current.text
   };
   const auto blockers = cobb::arrays::construct_from<QSignalBlocker>(widgets);

   auto        qmi  = _selected_log_entry_qmi();
   const auto* data = this->models.log_entries->logEntry(qmi);
   if (!data || !qmi.isValid()) {
      for (auto* widget : widgets)
         widget->setEnabled(false);
      this->ui.log_entries.current.complete->setChecked(false);
      this->ui.log_entries.current.conditions->clear();
      this->ui.log_entries.current.fail->setChecked(false);
      this->ui.log_entries.current.next_quest->setFormStub(nullptr);
      this->ui.log_entries.current.text->setPlainText({});
      return;
   }
   for (auto* widget : widgets)
      widget->setEnabled(true);
   this->ui.log_entries.current.complete->setChecked(data->complete_quest);
   this->ui.log_entries.current.fail->setChecked(data->fail_quest);
   this->ui.log_entries.current.next_quest->setFormStub(data->next_quest);
   this->ui.log_entries.current.text->setPlainText(data->text);
   {
      auto* widget = this->ui.log_entries.current.fragment;
      widget->setCurrentScriptname(data->fragment.scriptname);
      widget->setCurrentFunction(data->fragment.function);
   }
   {
      auto* widget = this->ui.log_entries.current.conditions;
      widget->clear();
      widget->importFrom(this->working_quest, data->conditions);
   }
}
void QuestTabStages::_on_log_entry_data_edited() {
   using model_type = QuestStageLogEntriesModel;

   auto qmi = _selected_log_entry_qmi();
   if (!qmi.isValid())
      return;

   QuestStagesModel::LogEntryData data;
   data.complete_quest = this->ui.log_entries.current.complete->isChecked();
   this->ui.log_entries.current.conditions->exportTo(this->working_quest, data.conditions);
   data.fail_quest     = this->ui.log_entries.current.fail->isChecked();
   data.fragment = {
      .scriptname = this->ui.log_entries.current.fragment->currentScriptname(),
      .function   = this->ui.log_entries.current.fragment->currentFunction(),
   };
   data.next_quest     = this->ui.log_entries.current.next_quest->formStub();
   data.text           = this->ui.log_entries.current.text->toPlainText();

   this->models.log_entries->setLogEntry(qmi, std::move(data));
}