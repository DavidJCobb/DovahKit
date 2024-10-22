#include "./FormSubdialogSceneDialogueAction.h"
#include <limits>
#include "dovah/forms/Topic.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/open_window_for_form.h"
#include "ui/utils/set_custom_context_menu.h"
#include "ui/utils/set_tableview_column_widths.h"
#include "../quest/QuestDialogueTopicInfosModel.h"

#include <QMessageBox>
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

FormSubdialogSceneDialogueAction::FormSubdialogSceneDialogueAction(QuestAllDialogueDatastore& ds, QWidget* parent) : FormSubdialogSceneActionBase(parent), dialogue_datastore(ds) {
   this->ui.setupUi(this);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->ui.loopMin->setRange(0, std::numeric_limits<float>::max());
   this->ui.loopMax->setRange(0, std::numeric_limits<float>::max());
   
   {
      auto* widget = this->ui.emotionType;
      widget->clear();
      widget->addItem(tr("Neutral",  "emotion"), (int)dovah::dialogue::emotion::neutral);
      widget->addItem(tr("Anger",    "emotion"), (int)dovah::dialogue::emotion::anger);
      widget->addItem(tr("Disgust",  "emotion"), (int)dovah::dialogue::emotion::disgust);
      widget->addItem(tr("Fear",     "emotion"), (int)dovah::dialogue::emotion::fear);
      widget->addItem(tr("Sad",      "emotion"), (int)dovah::dialogue::emotion::sad);
      widget->addItem(tr("Happy",    "emotion"), (int)dovah::dialogue::emotion::happy);
      widget->addItem(tr("Surprise", "emotion"), (int)dovah::dialogue::emotion::surprise);
      widget->addItem(tr("Puzzled",  "emotion"), (int)dovah::dialogue::emotion::puzzled);
   }
   this->ui.emotionValue->setRange(0, 100);

   this->info_model = new QuestDialogueTopicInfosModel(this);
   this->info_model->setDatastore(&this->dialogue_datastore);
   this->ui.infos->setModel(this->info_model);
   {
      auto& menu  = this->context_menu.menu;
      auto& items = this->context_menu.actions;
      ui::set_custom_context_menu(*this->ui.infos, menu);
      ui::set_tableview_column_widths(this->ui.infos, [](QHeaderView& header, const QFontMetrics& metrics) {
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

      items.create    = menu.addAction(tr("New..."));
      items.edit      = menu.addAction(tr("Edit..."));
      items.move_up   = menu.addAction(tr("Move Up"));
      items.move_down = menu.addAction(tr("Move Down"));
      items.remove    = menu.addAction(tr("Delete"));
      items.use_info  = menu.addAction(tr("Use Info"));
      
      QObject::connect(items.create,    &QAction::triggered, this, &FormSubdialogSceneDialogueAction::_info_button_new);
      QObject::connect(items.edit,      &QAction::triggered, this, &FormSubdialogSceneDialogueAction::_info_button_edit);
      QObject::connect(items.move_up,   &QAction::triggered, this, &FormSubdialogSceneDialogueAction::_info_button_move_up);
      QObject::connect(items.move_down, &QAction::triggered, this, &FormSubdialogSceneDialogueAction::_info_button_move_down);
      QObject::connect(items.remove,    &QAction::triggered, this, &FormSubdialogSceneDialogueAction::_info_button_delete);
      QObject::connect(items.use_info,  &QAction::triggered, this, [this]() {
         if (auto* stub = this->_selected_info())
            open_use_info_dialog_for_form(*stub);
      });
      QObject::connect(&menu, &QMenu::aboutToShow, this, [this, &items]() {
         bool  has_selection = this->_selected_info() != nullptr;

         auto& items = this->context_menu.actions;
         items.edit->setEnabled(has_selection);
         items.move_up->setEnabled(has_selection);
         items.move_down->setEnabled(has_selection);
         items.remove->setEnabled(has_selection);
         items.use_info->setEnabled(has_selection);
      });
   }

   QObject::connect(this, &QDialog::accepted, this, [this]() {
      this->data.emotion.type = (dovah::dialogue::emotion)this->ui.emotionType->currentData().toInt();
      this->data.emotion.value = this->ui.emotionValue->value();

      this->data.looping.enabled = this->ui.flagLooping->isChecked();
      this->data.looping.min = this->ui.loopMin->value();
      this->data.looping.max = this->ui.loopMax->value();

      this->data.headtrack.alias_id    = this->ui.headtrackAlias->currentData().toInt();
      this->data.headtrack.face_target = this->ui.headtrackTurn->isChecked();
      this->data.headtrack.at_player   = this->ui.headtrackUsePlayer->isChecked();

      if (auto* stub = this->data.topic) {
         auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Topic>();
         if (loaded) {
            if (this->ui.flagDoAllBeforeRepeating->isChecked())
               loaded->data.flags |= dovah::loaded_forms::Topic::dialogue_flag::do_all_before_repeating;
            else
               loaded->data.flags &= ~dovah::loaded_forms::Topic::dialogue_flag::do_all_before_repeating;
         }
      }
   });
}
void FormSubdialogSceneDialogueAction::refresh() {
   this->_refresh_base(
      this->ui.name,
      this->ui.actor,
      this->ui.phaseStart,
      this->ui.phaseEnd
   );

   this->ui.emotionType->setCurrentIndex(this->ui.emotionType->findData((int)this->data.emotion.type));
   this->ui.emotionValue->setValue(this->data.emotion.value);

   this->ui.flagLooping->setChecked(this->data.looping.enabled);
   this->ui.loopMin->setValue(this->data.looping.min);
   this->ui.loopMax->setValue(this->data.looping.max);

   this->_make_alias_picker(*this->ui.headtrackAlias, this->data.headtrack.alias_id);
   this->ui.headtrackTurn->setChecked(this->data.headtrack.face_target);
   if (this->data.headtrack.at_player) {
      this->ui.headtrackUsePlayer->setChecked(true);
   } else {
      this->ui.headtrackUseAlias->setChecked(true);
   }
   
   if (auto* stub = this->data.topic) {
      auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Topic>();
      if (loaded) {
         this->ui.flagDoAllBeforeRepeating->setChecked(loaded->data.flags & ~dovah::loaded_forms::Topic::dialogue_flag::do_all_before_repeating);
      }

      this->info_model->setRootTopic(stub);
   }
}

void FormSubdialogSceneDialogueAction::select_info(dovah::form_stub* stub) {
   auto* view = this->ui.infos;
   if (!stub) {
      view->selectionModel()->clear();
      return;
   }

   auto* parent = stub->get_parent_form();
   if (parent != this->data.topic)
      return;

   auto*  model = this->info_model;
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

dovah::form_stub* FormSubdialogSceneDialogueAction::_selected_info() const {
   auto* view      = this->ui.infos;
   auto* sel_model = view->selectionModel();
   auto  rows      = sel_model->selectedRows();
   if (rows.isEmpty())
      return nullptr;

   return this->info_model->data(rows[0], QuestDialogueTopicInfosModel::FormStubRole).value<dovah::form_stub*>();
}

void FormSubdialogSceneDialogueAction::_info_button_new() {
   auto* topic = this->data.topic;
   if (!topic)
      return;

   dovah::form_stub* info = nullptr;
   try {
      auto request = DovahKitCore::get().request_form_creation(dovah::form_type::topic_info);
      request.set_parent_form(topic);
      info = request.commit();
   } catch (const dovah::exceptions::form_creation_failed& ex) {
      _report_form_create_error(this, ex.code);
   }
   if (info) {
      this->select_info(info);
      open_edit_dialog_for_form(*info, this);
   }
}
void FormSubdialogSceneDialogueAction::_info_button_edit() {
   auto* stub = _selected_info();
   if (!stub)
      return;
   open_edit_dialog_for_form(*stub, this);
}
void FormSubdialogSceneDialogueAction::_info_button_move_up() {
   auto rows = this->ui.infos->selectionModel()->selectedRows();
   if (rows.isEmpty())
      return;
   auto* item = this->info_model->node(rows[0].row());
   if (!item)
      return;

   this->dialogue_datastore.reorder_info(*item, -1);
}
void FormSubdialogSceneDialogueAction::_info_button_move_down() {
   auto rows = this->ui.infos->selectionModel()->selectedRows();
   if (rows.isEmpty())
      return;
   auto* item = this->info_model->node(rows[0].row());
   if (!item)
      return;

   this->dialogue_datastore.reorder_info(*item, 1);
}
void FormSubdialogSceneDialogueAction::_info_button_delete() {
   auto* stub = _selected_info();
   if (!stub)
      return;

   DovahKitCore::get().delete_form(*stub, this);
}

void FormSubdialogSceneDialogueAction::showEvent(QShowEvent* event) {
   QDialog::showEvent(event);

   if (this->_did_first_show)
      return;
   this->_did_first_show = true;
   if (this->show_info_on_open && this->show_info_on_open->form_type == dovah::form_type::topic_info) {
      this->select_info(this->show_info_on_open);
      open_edit_dialog_for_form(*this->show_info_on_open, this);
   }
}