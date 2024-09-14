#include "./QuestDialogueTabBody.h"
#include <cassert>
#include <QInputDialog>
#include <QMessageBox>
#include "dovah/data/dialogue/topic_subtype.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_branch.h"
#include "dovah/forms/DialogueBranch.h"
#include "dovah/forms/Topic.h"
#include "dovah/forms/TopicInfo.h"
#include "editor/subsystems/game_settings/core.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/open_window_for_form.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/set_tableview_column_widths.h"
#include "ui/utils/typical_tableview_config.h"

#include "./QuestDialogueBranchesModel.h"
#include "./QuestDialogueBranchedTopicsModel.h"
#include "./QuestDialogueBranchlessTopicsModel.h"
#include "./QuestDialogueTopicInfosModel.h"

#include <QDialog>
#include <QGridLayout>
#include <QListWidget>
#include <QPushButton>
namespace {
   uint32_t _pick_subtype(QuestAllDialogueDatastore* ds, dovah::dialogue::category cat, QWidget* parent) {
      QDialog*     dialog = new QDialog(parent);
      QListWidget* list   = new QListWidget(dialog);
      {
         dialog->setWindowTitle(QuestDialogueTabBody::tr("Pick topic type"));
         auto* layout = new QGridLayout(dialog);
         layout->addWidget(list);

         auto* btn = new QHBoxLayout(dialog);
         layout->addLayout(btn, 1, 0);

         btn->addStretch(1);
         {
            auto* button = new QPushButton(QuestDialogueTabBody::tr("OK"), dialog);
            btn->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::accept);
         }
         {
            auto* button = new QPushButton(QuestDialogueTabBody::tr("OK"), dialog);
            btn->addWidget(button);
            QObject::connect(button, &QPushButton::clicked, dialog, &QDialog::reject);
         }
         btn->addStretch(1);
      }

      bool any_subtypes_available = false;
      {
         auto& existing_topics = ds->all_branchless_topics();

         auto& editor = DovahKitCore::get();
         auto& gss    = dovahkit::subsystems::game_settings::core::get();
         for (auto& dfn : dovah::dialogue::all_topic_subtypes) {
            if (dfn.category != cat)
               continue;

            bool used = false;
            for (auto* item : existing_topics) {
               if (item->cached.subtype == dfn.signature) {
                  used = true;
                  break;
               }
            }
            if (used)
               continue;

            QString name;
            {
               auto gs_name = dfn.game_setting_for_name();
               auto variant = gss.get_setting_value(gs_name.c_str());
               if (std::holds_alternative<dovah::localized_string>(variant)) {
                  name = editor.convert_localized_string(std::get<dovah::localized_string>(variant));
               }
               if (name.isEmpty()) {
                  variant = gss.get_setting_default_value(gs_name.c_str());
                  if (std::holds_alternative<dovah::localized_string>(variant)) {
                     name = editor.convert_localized_string(std::get<dovah::localized_string>(variant));
                  }
                  if (name.isEmpty()) {
                     name = QString::fromStdString(std::string(dfn.internal_name));
                  }
               }
            }

            auto* item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, dfn.signature);
            list->addItem(item);
            any_subtypes_available = true;
         }
      }
      if (!any_subtypes_available) {
         dialog->deleteLater();
         QMessageBox::critical(
            parent,
            QuestDialogueTabBody::tr("Error"),
            QuestDialogueTabBody::tr("All possible dialogue types have been used.")
         );
         return 0;
      }
      dialog->exec();
      dialog->deleteLater();

      uint32_t subtype = 0;
      {
         auto row = list->currentRow();
         if (row >= 0)
            if (auto* item = list->item(row))
               subtype = item->data(Qt::UserRole).toInt();
      }
      return subtype;
   }
}


#include "dovah/exceptions/form_creation_failed.h"
namespace {
   void _report_form_create_error(QWidget* window, const dovah::exceptions::form_creation_failed::error_code ec) {
      using error_code = std::decay_t<decltype(ec)>;
      //
      QString text;
      switch (ec) {
         case error_code::invalid_form_type:
            text = QObject::tr("An internal program error occurred: DovahKit tried to create a form but supplied a bad form type.");
            break;
         case error_code::no_active_file:
            text = QObject::tr("There is no active file, nor any room in the load order for a new file.");
            break;
         case error_code::no_form_id_available:
            text = QObject::tr("You've used up all of the form IDs available to this file!");
            break;
         case error_code::unimplemented_form_type:
            text = QObject::tr("DovahKit does not support editing this form type.");
            break;
         case error_code::invalid_parent_child_relationship:
            text = QObject::tr("The specified parent form cannot have a child form of this type. (Wait, what? How did you get the dialogue editor to try to do that?)");
            break;
         case error_code::exterior_grid_coordinates_already_taken:
            text = QObject::tr("The specified worldspace already has an exterior cell at the desired grid coordinates. (Wait, what? How did you get the dialogue editor to try and create an exterior cell?)");
            break;
         case error_code::cannot_create_reference_with_no_parent_cell:
            text = QObject::tr("References cannot be created outside of a cell. (Wait, what? How did you get the dialogue editor to try and create a reference?)");
            break;
         case error_code::interior_cell_clone_cannot_have_parent:
            text = QObject::tr("Interior cells cannot have a parent worldspace. (Wait, what? How did you get the dialogue editor to try and create an interior cell?)");
            break;
         case error_code::exterior_cell_clone_must_have_parent:
            text = QObject::tr("Exterior cells must have a parent worldspace. (Wait, what? How did you get the dialogue editor to try and create an exterior cell?)");
            break;
         case error_code::cannot_sever_references_to_none_stub:
            text = QObject::tr("DovahKit needed to select a form ID to use for the new form. The chosen form ID is the target of one or more dangling references, and DovahKit does not know how to sever those references, so the form creation process could not continue.");
            break;
      }
      QMessageBox::critical(
         window,
         QObject::tr("Error", "create new form error"),
         QObject::tr("Unable to create new form. %1").arg(text)
      );
   }
}

QuestDialogueTabBody::QuestDialogueTabBody(QWidget* parent) : QWidget(parent) {
   this->ui.setupUi(this);

   {
      auto* view  = this->ui.branches;
      auto* model = this->_models.branches = new QuestDialogueBranchesModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(QuestDialogueBranchesModel::Column::EditorID, 1, 0, 100);
         header.setColumnFlex(QuestDialogueBranchesModel::Column::FormID,   0, 0, 4);
         header.setColumnFlex(QuestDialogueBranchesModel::Column::Flags,    0, 0, metrics.boundingRect("XX").width() * 1.5F + 4);
      });

      auto* sel_model = view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &QuestDialogueTabBody::_branch_selection_changed);

      QObject::connect(this->ui.buttonBranchNew,    &QPushButton::clicked, this, &QuestDialogueTabBody::_branch_button_new);
      QObject::connect(this->ui.buttonBranchEdit,   &QPushButton::clicked, this, &QuestDialogueTabBody::_branch_button_edit);
      QObject::connect(this->ui.buttonBranchDelete, &QPushButton::clicked, this, &QuestDialogueTabBody::_branch_button_delete);
   }
   {
      auto* view = this->ui.topics;
      auto* model = this->_models.topics.branched = new QuestDialogueBranchedTopicsModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(QuestDialogueBranchedTopicsModel::Column::EditorID,    1, 0, 100);
         header.setColumnFlex(QuestDialogueBranchedTopicsModel::Column::FormID,      0, 0, 4);
         header.setColumnFlex(QuestDialogueBranchedTopicsModel::Column::DisplayText, 1, 0, 100);
         header.setColumnFlex(QuestDialogueBranchlessTopicsModel::Column::Subtype,   0, 0, metrics.boundingRect("CombatCombat").width() * 1.5F + 4);
         header.setColumnFlex(QuestDialogueBranchedTopicsModel::Column::Priority,    0, 0, metrics.boundingRect("100").width() * 1.5F + 4);
      });

      // Don't forget the alternate model, too!
      this->_models.topics.branchless = new QuestDialogueBranchlessTopicsModel(this);
      
      QObject::connect(this->ui.buttonTopicNew,    &QPushButton::clicked, this, &QuestDialogueTabBody::_topic_button_new);
      QObject::connect(this->ui.buttonTopicEdit,   &QPushButton::clicked, this, &QuestDialogueTabBody::_topic_button_edit);
      QObject::connect(this->ui.buttonTopicDelete, &QPushButton::clicked, this, &QuestDialogueTabBody::_topic_button_delete);
   }
   {
      auto* view = this->ui.infos;
      auto* model = this->_models.infos = new QuestDialogueTopicInfosModel(this);
      view->setModel(model);
      ui::typical_tableview_config(view);
      ui::set_tableview_column_widths(view, [](QHeaderView& header, const QFontMetrics& metrics) {
         header.resizeSection(QuestDialogueTopicInfosModel::Column::InfoText,        200);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::EditorID,        4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::FormID,          4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::Flags,           metrics.boundingRect("EEEEEO(1.00)").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::ResponseCount,   metrics.boundingRect("10").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::Speaker,         metrics.boundingRect("Protagonist").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::Target,          metrics.boundingRect("Protagonist").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::InFaction,       metrics.boundingRect("Protagonist").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::IsVoiceType,     metrics.boundingRect("FemaleEvenToned").width() * 1.5F + 4);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::Conditions,      100);
         header.resizeSection(QuestDialogueTopicInfosModel::Column::HasResultScript, metrics.boundingRect("Y").width() * 1.5F + 4);
      });

      auto* sel_model = view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &QuestDialogueTabBody::_info_selection_changed);
      
      QObject::connect(this->ui.buttonInfoNew,      &QPushButton::clicked, this, &QuestDialogueTabBody::_info_button_new);
      QObject::connect(this->ui.buttonInfoEdit,     &QPushButton::clicked, this, &QuestDialogueTabBody::_info_button_edit);
      QObject::connect(this->ui.buttonInfoMoveUp,   &QPushButton::clicked, this, &QuestDialogueTabBody::_info_button_move_up);
      QObject::connect(this->ui.buttonInfoMoveDown, &QPushButton::clicked, this, &QuestDialogueTabBody::_info_button_move_down);
      QObject::connect(this->ui.buttonInfoDelete,   &QPushButton::clicked, this, &QuestDialogueTabBody::_info_button_delete);
   }

   // Do this last, as signals triggered will try to access the other models.
   this->_set_up_topic_selection_model();
}

QuestAllDialogueDatastore* QuestDialogueTabBody::datastore() const {
   //
   // NOTE: Avoid using the "branches" model for this. We clear it when showing 
   //       branchless categories by setting its datastore to nullptr.
   //
   return this->_models.infos->datastore();
}

void QuestDialogueTabBody::setCategory(dovah::dialogue::category c) {
   if (c == this->_category)
      return;
   this->_category = c;

   if (c == dovah::dialogue::category::topic) {
      this->_models.branches->setDatastore(this->datastore());
      this->ui.branches->setEnabled(true);
      this->ui.buttonBranchNew->setEnabled(true);
      this->ui.buttonBranchEdit->setEnabled(false);
      this->ui.buttonBranchDelete->setEnabled(false);

      this->ui.topics->setModel(this->_models.topics.branched);

      this->_reset_selection_of(this->ui.branches);
      this->_branch_selection_changed(); // just in case; see other situation where sel-changed funcs needed to be called manually
   } else {
      this->_models.branches->setDatastore(nullptr);
      this->ui.branches->setEnabled(false);
      this->ui.buttonBranchNew->setEnabled(false);
      this->ui.buttonBranchEdit->setEnabled(false);
      this->ui.buttonBranchDelete->setEnabled(false);

      this->ui.topics->setModel(this->_models.topics.branchless);
      this->_models.topics.branchless->setCategory(c);
   }
   this->_set_up_topic_selection_model();
}
void QuestDialogueTabBody::setDatastore(QuestAllDialogueDatastore* ds) {
   if (this->_category == dovah::dialogue::category::topic) {
      this->_models.branches->setDatastore(ds);
   }
   this->_models.topics.branched->setDatastore(ds);
   this->_models.topics.branchless->setDatastore(ds);
   this->_models.infos->setDatastore(ds);
}

void QuestDialogueTabBody::_set_up_topic_selection_model() {
   auto* sel_model = this->ui.topics->selectionModel();
   QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, &QuestDialogueTabBody::_topic_selection_changed);
   this->_topic_selection_changed();
}

void QuestDialogueTabBody::_reset_selection_of(QTableView* view) {
   view->selectionModel()->clear();
}

void QuestDialogueTabBody::_branch_selection_changed() {
   dovah::form_stub* stub = this->selected_branch();

   bool stub_is_valid = stub != nullptr;
   this->ui.buttonBranchEdit->setEnabled(stub_is_valid);
   this->ui.buttonBranchDelete->setEnabled(stub_is_valid);
   this->ui.buttonTopicNew->setEnabled(stub_is_valid);

   this->_models.topics.branched->setRootBranch(stub);
   _reset_selection_of(this->ui.topics);
   _topic_selection_changed(); // I guess if the underlying model gets reset it doesn't trigger a selection change?! so we gotta fire it manually. Ugh...
}
void QuestDialogueTabBody::_topic_selection_changed() {
   dovah::form_stub* stub = this->selected_topic();

   bool stub_is_valid = stub != nullptr;
   this->ui.buttonTopicEdit->setEnabled(stub_is_valid);
   this->ui.buttonTopicDelete->setEnabled(stub_is_valid);
   this->ui.buttonInfoNew->setEnabled(stub_is_valid);

   this->_models.infos->setRootTopic(stub);
   _reset_selection_of(this->ui.infos);
   _info_selection_changed(); // I guess if the underlying model gets reset it doesn't trigger a selection change?! so we gotta fire it manually. Ugh...
}
void QuestDialogueTabBody::_info_selection_changed() {
   dovah::form_stub* stub = this->selected_info();

   bool stub_is_valid = stub != nullptr;
   this->ui.buttonInfoEdit->setEnabled(stub_is_valid);
   this->ui.buttonInfoMoveUp->setEnabled(stub_is_valid);
   this->ui.buttonInfoMoveDown->setEnabled(stub_is_valid);
   this->ui.buttonInfoDelete->setEnabled(stub_is_valid);
}

dovah::form_stub* QuestDialogueTabBody::_quest() const {
   if (auto* ds = this->datastore())
      return ds->get_quest();
   return nullptr;
}

dovah::form_stub* QuestDialogueTabBody::_spawn_branch(QString branch_editor_id, QString topic_editor_id) {
   auto* quest = this->_quest();
   assert(quest != nullptr);

   auto& editor = DovahKitCore::get();

   dovah::form_stub* branch = nullptr;
   dovah::form_stub* topic  = nullptr;
   //
   // Create the branch:
   //
   {
      auto request = editor.request_form_creation(dovah::form_type::dialogue_branch);
      request.editorID = branch_editor_id.toStdString();
      branch = request.commit();
   }
   auto b_loaded = branch->load().ptr_cast<dovah::loaded_forms::DialogueBranch>();
   assert(b_loaded != nullptr);
   {
      emit editor.formModificationImminent(branch);
      b_loaded->owning_quest.set(*b_loaded, quest);
      emit editor.formModified(branch);
   }
   //
   // Create the topic:
   //
   topic = this->_spawn_topic(topic_editor_id, branch, 'CUST');
   {
      emit editor.formModificationImminent(branch);
      b_loaded->starting_topic.set(*b_loaded, topic);
      emit editor.formModified(branch);
   }
   return branch;
}
dovah::form_stub* QuestDialogueTabBody::_spawn_topic(QString editor_id, dovah::form_stub* branch, uint32_t subtype_signature) {
   auto* quest = this->_quest();
   assert(quest != nullptr);

   auto& editor = DovahKitCore::get();

   auto request = editor.request_form_creation(dovah::form_type::topic);
   request.editorID = editor_id.toStdString();
   auto* topic = request.commit();

   auto t_loaded = branch->load().ptr_cast<dovah::loaded_forms::Topic>();
   assert(t_loaded != nullptr);
   {
      emit editor.formModificationImminent(branch);
      {
         for (size_t i = 0; i < dovah::dialogue::all_topic_subtypes.size(); ++i) {
            if (dovah::dialogue::all_topic_subtypes[i].signature == subtype_signature) {
               t_loaded->data.subtype = i;
               break;
            }
         }
         t_loaded->data.category = this->_category;
         t_loaded->subtype       = subtype_signature;
      }
      t_loaded->owning_forms.quest.set(*t_loaded,  quest);
      t_loaded->owning_forms.branch.set(*t_loaded, branch);
      emit editor.formModified(branch);
   }
   return topic;
}
dovah::form_stub* QuestDialogueTabBody::_spawn_info(dovah::form_stub* topic) {
   auto* quest = this->_quest();
   assert(quest != nullptr);

   auto& editor = DovahKitCore::get();

   auto request = editor.request_form_creation(dovah::form_type::topic_info);
   request.set_parent_form(topic);
   auto* info = request.commit();

   return info;
}


#pragma region UI button handlers
   #pragma region Branches
      void QuestDialogueTabBody::_branch_button_new() {
         dovah::form_stub* quest;
         if (auto* ds = this->datastore())
            quest = ds->get_quest();
         if (!quest)
            return;

         QString branch_editor_id;
         QString topic_editor_id;
         {
            bool ok = false;
            branch_editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID for new branch:"), QLineEdit::Normal, "", &ok);
            if (!ok)
               return;
            topic_editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID for new topic:"), QLineEdit::Normal, "", &ok);
            if (!ok)
               return;
         }
         dovah::form_stub* branch_stub = nullptr;
         dovah::form_stub* topic_stub  = nullptr;
         try {
            branch_stub = this->_spawn_branch(branch_editor_id, topic_editor_id);
         } catch (const dovah::exceptions::form_creation_failed& ex) {
            _report_form_create_error(this, ex.code);
         }
      }
      void QuestDialogueTabBody::_branch_button_edit() {
         auto* stub = selected_branch();
         if (!stub)
            return;
         //open_edit_dialog_for_form(*stub, this);
         open_edit_dialog_for_form(*stub);
      }
      void QuestDialogueTabBody::_branch_button_delete() {
         auto* stub = selected_branch();
         if (!stub)
            return;
         DovahKitCore::get().delete_form(*stub, this);
      }
   #pragma endregion
   #pragma region Topics
      void QuestDialogueTabBody::_topic_button_new() {
         auto* branch = this->selected_branch();
         if (!branch)
            return;

         uint32_t subtype = 0;
         switch (this->_category) {
            case dovah::dialogue::category::topic:
               subtype = 'CUST';
               break;
            case dovah::dialogue::category::scene:
               subtype = 'SCEN';
               break;
            default:
               subtype = _pick_subtype(this->datastore(), this->_category, this);
               if (!subtype)
                  return;
               break;
         }

         QString topic_editor_id;
         {
            bool ok = false;
            topic_editor_id = QInputDialog::getText(this, tr("Set editor ID"), tr("Editor ID for new topic:"), QLineEdit::Normal, "", &ok);
            if (!ok)
               return;
         }
         try {
            this->_spawn_topic(topic_editor_id, branch, subtype);
         } catch (const dovah::exceptions::form_creation_failed& ex) {
            _report_form_create_error(this, ex.code);
         }
      }
      void QuestDialogueTabBody::_topic_button_edit() {
         auto* stub = selected_topic();
         if (!stub)
            return;
         open_edit_dialog_for_form(*stub, this);
      }
      void QuestDialogueTabBody::_topic_button_delete() {
         auto* stub = selected_topic();
         if (!stub)
            return;
         DovahKitCore::get().delete_form(*stub, this);
      }
   #pragma endregion
   #pragma region Infos
      void QuestDialogueTabBody::_info_button_new() {
         auto* topic = this->selected_topic();
         if (!topic)
            return;

         try {
            this->_spawn_info(topic);
         } catch (const dovah::exceptions::form_creation_failed& ex) {
            _report_form_create_error(this, ex.code);
         }
      }
      void QuestDialogueTabBody::_info_button_edit() {
         auto* stub = selected_info();
         if (!stub)
            return;
         open_edit_dialog_for_form(*stub, this);
      }
      void QuestDialogueTabBody::_info_button_move_up() {
         auto* ds = this->datastore();
         if (!ds)
            return;

         auto rows = this->ui.infos->selectionModel()->selectedRows();
         if (rows.isEmpty())
            return;
         auto* item = this->_models.infos->node(rows[0].row());
         if (!item)
            return;

         ds->reorder_info(*item, -1);
      }
      void QuestDialogueTabBody::_info_button_move_down() {
         auto* ds = this->datastore();
         if (!ds)
            return;

         auto rows = this->ui.infos->selectionModel()->selectedRows();
         if (rows.isEmpty())
            return;
         auto* item = this->_models.infos->node(rows[0].row());
         if (!item)
            return;

         ds->reorder_info(*item, 1);
      }
      void QuestDialogueTabBody::_info_button_delete() {
         auto* stub = selected_info();
         if (!stub)
            return;

         DovahKitCore::get().delete_form(*stub, this);
      }
   #pragma endregion
#pragma endregion

dovah::form_stub* QuestDialogueTabBody::selected_branch() const {
   auto rows = this->ui.branches->selectionModel()->selectedRows();
   if (rows.isEmpty())
      return nullptr;
   return this->_models.branches->data(rows[0], QuestDialogueBranchesModel::FormStubRole).value<dovah::form_stub*>();
}
dovah::form_stub* QuestDialogueTabBody::selected_topic() const {
   auto* view = this->ui.topics;
   auto  rows = view->selectionModel()->selectedRows();
   if (rows.isEmpty())
      return nullptr;
   return view->model()->data(rows[0], Qt::UserRole).value<dovah::form_stub*>();
}
dovah::form_stub* QuestDialogueTabBody::selected_info() const {
   auto rows = this->ui.infos->selectionModel()->selectedRows();
   if (rows.isEmpty())
      return nullptr;
   return this->_models.infos->data(rows[0], QuestDialogueTopicInfosModel::FormStubRole).value<dovah::form_stub*>();
}

void QuestDialogueTabBody::select_branch(dovah::form_stub* stub) {
   auto* view  = this->ui.branches;
   auto* model = this->_models.branches;
   if (!stub) {
      this->_reset_selection_of(view);
      this->_branch_selection_changed();
      return;
   }

   size_t i = model->index_of(*stub);
   if (i == (size_t)-1)
      return;

   view->selectionModel()->select(
      {
         model->index(i, 0, {}),
         model->index(i, model->columnCount() - 1, {})
      },
      QItemSelectionModel::SelectionFlag::ClearAndSelect
   );
}
void QuestDialogueTabBody::select_topic(dovah::form_stub* stub) {
   auto* view = this->ui.topics;
   if (!stub) {
      this->_reset_selection_of(view);
      this->_topic_selection_changed();
      return;
   }

   QModelIndex tl;
   QModelIndex br;
   if (this->_category == dovah::dialogue::category::topic) {
      auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
      if (!branch_stub)
         return;
      this->select_branch(branch_stub);

      auto*  model = this->_models.topics.branched;
      size_t i     = model->index_of(*stub);
      if (i == (size_t)-1)
         return;
      tl = model->index(i, 0, {});
      br = model->index(i, model->columnCount() - 1, {});
   } else {
      auto* branch_stub = dovah::form_stub_helpers::get_dialogue_topic_branch(stub);
      if (branch_stub)
         return;
      
      auto*  model = this->_models.topics.branchless;
      size_t i     = model->index_of(*stub);
      if (i == (size_t)-1)
         return;
      tl = model->index(i, 0, {});
      br = model->index(i, model->columnCount() - 1, {});
   }
   view->selectionModel()->select(
      { tl, br },
      QItemSelectionModel::SelectionFlag::ClearAndSelect
   );
}
void QuestDialogueTabBody::select_info(dovah::form_stub* stub) {
   auto* view = this->ui.infos;
   if (!stub) {
      this->_reset_selection_of(view);
      this->_info_selection_changed();
      return;
   }

   auto* parent = stub->get_parent_form();
   if (parent && parent->form_type != dovah::form_type::topic)
      parent = nullptr;
   this->select_topic(parent);
   if (!parent)
      return;

   auto*  model = this->_models.infos;
   size_t i     = model->index_of(*stub);
   if (i == (size_t)-1)
      return;

   view->selectionModel()->select(
      {
         model->index(i, 0, {}),
         model->index(i, model->columnCount() - 1, {})
      },
      QItemSelectionModel::SelectionFlag::ClearAndSelect
   );
}