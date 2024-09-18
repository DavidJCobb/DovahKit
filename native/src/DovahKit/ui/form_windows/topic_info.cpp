#include "./topic_info.h"
#include <limits>
#include <QKeyEvent>
#include "helpers/vectors/move_item_within.h"
#include "dovah/core.h"
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/components/papyrus/fragment_data/topic_info_fragment_data.h"
#include "dovah/forms/Topic.h"
#include "editor/core.h"
#include "editor/open_window_for_form.h"
#include "ui/utils/bind.h"
#include "ui/utils/typical_tableview_config.h"
#include "./topic_info/topic_info_response.h"
#include "./topic_info/TopicInfoLinkedTopicsModel.h"
#include "./topic_info/TopicInfoResponseTableviewModel.h"
#include "./topic_info/TopicInfoSharedInfoFormFilter.h"
#include "./topic_info/TopicInfoWalkAwayTopicFormFilter.h"

#include "widgets/DKFormPickerDialog.h"
#include "./topic_info/TopicInfoLinkedTopicsFormFilter.h"

FormDialogTopicInfo::FormDialogTopicInfo(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* view  = this->ui.responses;
      auto* model = this->_models.responses = new TopicInfoResponseTableviewModel(this);
      view->setModel(model);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      ui::typical_tableview_config(view);

      view->installEventFilter(this); // Delete key
      QObject::connect(view, &QAbstractItemView::doubleClicked, this, &FormDialogTopicInfo::_edit_response);
      {
         auto& menu  = this->_context_menus.responses.menu;
         auto& items = this->_context_menus.responses.actions;
         {
            auto* action = items.create = menu.addAction(tr("New..."));
            QObject::connect(action, &QAction::triggered, this, &FormDialogTopicInfo::_create_response);
         }
         {
            auto* action = items.edit = menu.addAction(tr("Edit..."));
            QObject::connect(action, &QAction::triggered, this, [this]() { this->_edit_response(); });
         }
         {
            auto* action = items.move_up = menu.addAction(tr("Move Up"));
            QObject::connect(action, &QAction::triggered, this, [this]() { this->_move_response(-1); });
         }
         {
            auto* action = items.move_down = menu.addAction(tr("Move Down"));
            QObject::connect(action, &QAction::triggered, this, [this]() { this->_move_response(1); });
         }
         {
            auto* action = items.remove = menu.addAction(tr("Delete"));
            QObject::connect(action, &QAction::triggered, this, &FormDialogTopicInfo::_delete_response);
         }
         QObject::connect(&menu, &QMenu::aboutToShow, this, [this, view, &menu, &items]() {
            if (this->form->use_shared_info) {
               items.create->setEnabled(false);
               items.edit->setEnabled(false);
               items.move_up->setEnabled(false);
               items.move_down->setEnabled(false);
               items.remove->setEnabled(false);
               return;
            }
            items.create->setEnabled(true);

            auto* sel_model = view->selectionModel();
            auto  qmi       = sel_model->currentIndex();
            if (qmi.isValid()) {
               size_t row = qmi.row();
               items.edit->setEnabled(true);
               items.move_up->setEnabled(row > 0);
               items.move_down->setEnabled(row + 1 < view->model()->rowCount());
               items.remove->setEnabled(true);
            } else {
               items.edit->setEnabled(false);
               items.move_up->setEnabled(false);
               items.move_down->setEnabled(false);
               items.remove->setEnabled(false);
            }
         });
      }
   }
   {  // SharedInfo selection
      auto* filter = this->_filters.shared_info = new TopicInfoSharedInfoFormFilter(this);
      auto* picker = this->ui.sharedInfo;
      QObject::connect(this->ui.sharedInfoSearch, &QLineEdit::textChanged, this, [this](QString v) {
         this->_filters.shared_info->setEditorIDFilter(v.toStdString());
      });

      picker->setAllowedFormType(dovah::form_type::topic_info);
      picker->setCustomFilter(filter);

      this->ui.buttonEditSharedInfo->setEnabled(false);
      QObject::connect(picker, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
         this->ui.buttonEditSharedInfo->setEnabled(stub != nullptr);
      });
      QObject::connect(this->ui.buttonEditSharedInfo, &QPushButton::clicked, this, [this]() {
         auto* stub = this->form->use_shared_info.get_form_stub();
         if (!stub)
            return;
         open_edit_dialog_for_form(*stub, this->parentWidget());
      });
   }
   {
      auto* view  = this->ui.linkedTopics;
      auto* model = this->_models.linked_topics = new TopicInfoLinkedTopicsModel(this);
      view->setModel(model);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      ui::typical_tableview_config(view);

      {
         auto* filter = this->_filters.new_linked_topic = new TopicInfoLinkedTopicsFormFilter(this);
         filter->setModel(model);
      }

      view->installEventFilter(this); // Delete key
      {
         auto& menu  = this->_context_menus.linked_topics.menu;
         auto& items = this->_context_menus.linked_topics.actions;
         {
            auto* action = items.create = menu.addAction(tr("Add Topic..."));
            QObject::connect(action, &QAction::triggered, this, &FormDialogTopicInfo::_add_linked_topic);
         }
         {
            auto* action = items.move_up = menu.addAction(tr("Move Up"));
            QObject::connect(action, &QAction::triggered, this, [this]() { this->_move_linked_topic(-1); });
         }
         {
            auto* action = items.move_down = menu.addAction(tr("Move Down"));
            QObject::connect(action, &QAction::triggered, this, [this]() { this->_move_linked_topic(1); });
         }
         {
            auto* action = items.remove = menu.addAction(tr("Remove"));
            QObject::connect(action, &QAction::triggered, this, &FormDialogTopicInfo::_remove_linked_topic);
         }
         QObject::connect(&menu, &QMenu::aboutToShow, this, [this, view, &menu, &items]() {
            auto* sel_model = view->selectionModel();
            auto  qmi       = sel_model->currentIndex();
            if (qmi.isValid()) {
               size_t row = qmi.row();
               items.move_up->setEnabled(row > 0);
               items.move_down->setEnabled(row + 1 < view->model()->rowCount());
               items.remove->setEnabled(true);
            } else {
               items.move_up->setEnabled(false);
               items.move_down->setEnabled(false);
               items.remove->setEnabled(false);
            }
         });
      }
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
         this->ui.walkAwayGroupbox->setEnabled(true);
      });
      QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this, model]() {
         if (model->rowCount() == 0)
            this->ui.walkAwayGroupbox->setEnabled(false);
      });

      {
         auto* picker = this->ui.walkAwayTopic;
         auto* filter = this->_filters.walk_away_topic = new TopicInfoWalkAwayTopicFormFilter(this);
         filter->setSourceModel(this->_models.linked_topics);

         picker->setAllowedFormType(dovah::form_type::topic);
         picker->setCustomFilter(filter);
      }
   }
   
   this->ui.speaker->setAllowedFormType(dovah::form_type::actor_base);
   this->ui.audioOutputOverride->setAllowedFormType(dovah::form_type::sound_output_model);
   {
      auto* widget = this->ui.favorLevel;
      widget->clear();
      widget->addItem(tr("None"),   (int)loaded_form_type::favor_level_t::none);
      widget->addItem(tr("Small"),  (int)loaded_form_type::favor_level_t::small);
      widget->addItem(tr("Medium"), (int)loaded_form_type::favor_level_t::medium);
      widget->addItem(tr("Large"),  (int)loaded_form_type::favor_level_t::large);
   }
   this->ui.hoursUntilReset->setRange(0, 24);

   {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         if (stub != this->formStub()->get_parent_form())
            return;
         this->_update_topic_text_preview();
      });
   }

   this->load(); // this creates the working copy.

   this->_update_topic_text_preview();
}
void FormDialogTopicInfo::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_update_topic_text_preview();
   {
      dovah::form_stub* owning_quest = nullptr;
      if (auto* topic = this->formStub()->get_parent_form(); topic && topic->form_type == dovah::form_type::topic) {
         owning_quest = dovah::form_stub_helpers::get_dialogue_topic_quest(topic);
      }

      this->_filters.new_linked_topic->setOwningQuest(owning_quest);
      this->_filters.shared_info->setOwningQuest(owning_quest);
   }

   //ui::bind(this->ui.editorID, this->editor_id());
   this->ui.prompt->setText(editor.convert_localized_string(working.override_topic_text));

   {
      ui::bind(this->ui.speaker,             working.speaker, working);
      ui::bind(this->ui.audioOutputOverride, working.audio_output_override, working);
   }
   {  // Responses editor
      ui::bind(this->ui.sharedInfo, working.use_shared_info, working);
      {
         std::vector<TopicInfoResponseTableviewModel::node_type> nodes;

         dovah::loaded_form_ptr<loaded_form_type> src_form;
         std::vector<loaded_form_type::response>* src_list = &working.responses;
         if (auto* stub = working.use_shared_info.get_form_stub()) {
            src_form = stub->load().ptr_cast<loaded_form_type>();
            if (src_form)
               src_list = &src_form->responses;
         }

         for (auto& src : *src_list) {
            auto& node = nodes.emplace_back();
            node.edited  = false;
            node.emotion = {
               .type  = src.emotion.type,
               .value = src.emotion.value,
            };
            node.response_text = editor.convert_localized_string(src.text);
         }
         this->_models.responses->overwriteAllItems(nodes);
      }
   }
   {  // Flags and similar
      auto& flags = working.info_flags;
      ui::bind_inverse(this->ui.flagHasLIPFile, flags, loaded_form_type::info_flag::no_lip_file);
      ui::bind(this->ui.flagForceSubtitles, flags, loaded_form_type::info_flag::force_subtitle);
      ui::bind(this->ui.flagSayOnce, flags, loaded_form_type::info_flag::say_once);
      ui::bind(this->ui.flagGoodbye, flags, loaded_form_type::info_flag::goodbye);
      ui::bind(this->ui.flagRandom, flags, loaded_form_type::info_flag::random);
      ui::bind(this->ui.flagRandomEnd, flags, loaded_form_type::info_flag::random_end);
      ui::bind(this->ui.flagCanMoveWhileGreeting, flags, loaded_form_type::info_flag::can_move_while_greeting);
      ui::bind(this->ui.flagReqPostprocess, flags, loaded_form_type::info_flag::requires_post_processing);
      ui::bind(this->ui.flagSpendsFavorPoints, flags, loaded_form_type::info_flag::spends_favor_points);
      ui::bind(this->ui.favorLevel, working.favor_level);
      {
         auto* widget = this->ui.hoursUntilReset;
         widget->setValue(working.get_hours_until_reset());
         QObject::connect(widget, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double v) {
            this->form->set_hours_until_reset(v);
         });
      }
   }
   this->ui.conditions->importBifurcatedList(working, working.conditions.locked, working.conditions.normal);
   {  // Link to Topics
      ui::bind(this->ui.flagInvisContinue, working.info_flags, loaded_form_type::info_flag::invisible_continue);
      this->_models.linked_topics->importFrom(working.link_to.locked, working.link_to.normal);
      ui::bind(this->ui.walkAwayTopic, working.walk_away_topic, working);
      ui::bind(this->ui.flagWalkAwayTopicInvis, working.info_flags, loaded_form_type::info_flag::walk_away_invisible_in_menu);
   }
   {  // Scripts
      {
         auto* fragdata = (papyrus_fragment_data_type*) working.script_data.fragment_data;
         if (fragdata) {
            assert(fragdata->type == dovah::loaded_forms::components::papyrus::fragment_type::info);

            if (auto& opt = fragdata->fragments.on_begin; opt.has_value()) {
               auto& src = opt.value();
               auto* dst = this->ui.fragmentBegin;
               dst->setCurrentScriptname(src.script);
               dst->setCurrentFunction(src.function);
               if (src.script.empty())
                  dst->setCurrentScriptname(fragdata->filename);
            }
            if (auto& opt = fragdata->fragments.on_end; opt.has_value()) {
               auto& src = opt.value();
               auto* dst = this->ui.fragmentEnd;
               dst->setCurrentScriptname(src.script);
               dst->setCurrentFunction(src.function);
               if (src.script.empty())
                  dst->setCurrentScriptname(fragdata->filename);
            }
         }
      }
      this->ui.scriptListPane->setFormWorkingCopy(&working);
   }
}
void FormDialogTopicInfo::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   if (working.audio_output_override) {
      working.info_flags |= loaded_form_type::info_flag::audio_output_override;
   } else {
      working.info_flags &= ~loaded_form_type::info_flag::audio_output_override;
   }
   
   editor.assign_localized_string(working.override_topic_text, this->ui.prompt->text());
   this->ui.conditions->exportBifurcatedList(working, working.conditions.locked, working.conditions.normal);
   {  // Link To list
      this->_models.linked_topics->exportTo(working.link_to.normal, working);
   }
   {  // Scripts
      {  // Fragments
         using fragment_optional_type = decltype(decltype(papyrus_fragment_data_type::fragments)::on_begin);

         bool any_data = false;
         {
            auto* widget_a = this->ui.fragmentBegin;
            auto* widget_b = this->ui.fragmentEnd;
            if (!widget_a->currentScriptname().isEmpty())
               any_data = true;
            else if (!widget_b->currentScriptname().isEmpty())
               any_data = true;
            else if (!widget_a->currentFunction().isEmpty())
               any_data = true;
            else if (!widget_b->currentFunction().isEmpty())
               any_data = true;
         }

         auto* fragdata = (papyrus_fragment_data_type*)working.script_data.fragment_data;
         if (fragdata) {
            assert(fragdata->type == dovah::loaded_forms::components::papyrus::fragment_type::info);
         } else if (any_data) {
            working.script_data.fragment_data = new papyrus_fragment_data_type;
         }
         if (fragdata) {
            auto _write = [](const DKPapyrusFragmentFunctionPicker* src, fragment_optional_type& dst, std::string& dst_shared_filename) {
               auto scriptname = src->currentScriptname();
               auto function   = src->currentFunction();
               if (scriptname.isEmpty() && function.isEmpty()) {
                  dst = {};
               } else {
                  auto& dst_data = dst.emplace();
                  dst_data.script   = scriptname.toStdString();
                  dst_data.function = function.toStdString();

                  if (!dst_data.script.empty())
                     dst_shared_filename = dst_data.script;
               }
            };
            _write(this->ui.fragmentBegin, fragdata->fragments.on_begin, fragdata->filename);
            _write(this->ui.fragmentEnd,   fragdata->fragments.on_end,   fragdata->filename);
         }
      }
      this->ui.scriptListPane->commit();
   }
}

void FormDialogTopicInfo::_update_topic_text_preview() {
   auto* widget = this->ui.topicText;
   auto* topic  = this->formStub()->get_parent_form();
   if (!topic) {
      widget->setText("");
      return;
   }
   auto loaded = topic->load().ptr_cast<dovah::loaded_forms::Topic>();
   if (loaded) {
      widget->setText(DovahKitCore::get().convert_localized_string(loaded->text));
   } else {
      widget->setText("");
   }
}

#pragma region Handlers: Linkedtopics
   void FormDialogTopicInfo::_add_linked_topic() {
      auto* dialog = new DKFormPickerDialog(this);
      dialog->setCustomFilter(this->_filters.new_linked_topic);
      auto  result = dialog->exec();
      if (result == QDialog::Rejected)
         return;

      this->_models.linked_topics->addTopic(dialog->formStub());
      dialog->deleteLater();
   }
   void FormDialogTopicInfo::_move_linked_topic(int by) {
      auto rows = this->ui.linkedTopics->selectionModel()->selectedRows();
      if (rows.isEmpty())
         return;

      size_t row = rows[0].row();
      this->_models.linked_topics->reorderTopic(row, by);
   }
   void FormDialogTopicInfo::_remove_linked_topic() {
      auto* widget    = this->ui.linkedTopics;
      auto* model     = this->_models.linked_topics;
      auto* sel_model = widget->selectionModel();

      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty()) {
         size_t row = rows[0].row();
         model->removeTopic(row);
      }
   }
#pragma endregion

#pragma region Handlers: Responses
   void FormDialogTopicInfo::_create_response() {
      if (this->form->use_shared_info)
         return;

      auto* widget    = this->ui.responses;
      auto* model     = this->_models.responses;
      auto* sel_model = widget->selectionModel();

      auto added = model->create();
      if (!added.has_value())
         return;

      auto tl = model->index(added.value(), 0, {});
      auto br = model->index(added.value(), model->column_count - 1, {});
      sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
   }
   void FormDialogTopicInfo::_edit_response(const QModelIndex& qmi) {
      if (this->form->use_shared_info)
         return;

      size_t row;
      if (qmi.isValid()) {
         row = qmi.row();
      } else {
         auto* sel_model = this->ui.responses->selectionModel();
         auto  sel_qmi   = sel_model->currentIndex();
         if (sel_qmi.isValid()) {
            row = sel_qmi.row();
         } else {
            return;
         }
      }

      auto& list = this->form->responses;
      if (row >= list.size())
         return;
      auto& response = list[row];

      auto* dialog = new FormSubdialogTopicInfoResponse(this);
      dialog->importFrom(*this->form, response);
      auto  result = dialog->exec();
      if (result == QDialog::Rejected)
         return;
      dialog->exportTo(*this->form, response);
      //
      // And update our listview:
      //
      TopicInfoResponseTableviewModel::node_type node;
      node.edited  = true;
      node.emotion = {
         .type  = response.emotion.type,
         .value = response.emotion.value,
      };
      node.response_text = DovahKitCore::get().convert_localized_string(response.text);
      this->_models.responses->overwrite(row, node);
   }
   void FormDialogTopicInfo::_move_response(int by) {
      if (this->form->use_shared_info)
         return;

      if (by == 0)
         return;

      auto rows = this->ui.responses->selectionModel()->selectedRows();
      if (rows.isEmpty())
         return;

      auto* model = this->_models.responses;
      auto& list  = this->form->responses;

      size_t row = rows[0].row();
      if (by < 0) {
         if (row < -by)
            return;
      } else {
         if (row + (size_t)by >= list.size())
            return;
      }
      bool succeeded = cobb::vectors::move_item_within<false>(list, row, by);
      assert(succeeded);
      model->moveItem(model->index(row, 0, {}), by);
   }
   void FormDialogTopicInfo::_delete_response() {
      if (this->form->use_shared_info)
         return;

      auto* widget    = this->ui.responses;
      auto* model     = this->_models.responses;
      auto* sel_model = widget->selectionModel();

      auto rows = sel_model->selectedRows();
      if (!rows.isEmpty()) {
         size_t row = rows[0].row();
         {
            auto& working = *this->form;
            auto& list    = working.responses;
            list[row].clear(working);
            list.erase(list.begin() + row);
         }
         model->deleteItems(row, 1);
      }
   }
#pragma endregion

/*virtual*/ bool FormDialogTopicInfo::eventFilter(QObject* object, QEvent* event) /*override*/ {
   if (event->type() == QEvent::Type::KeyPress) {
      if (((QKeyEvent*)event)->key() == Qt::Key_Delete) {
         //
         // Handle Delete key.
         //
         if (object == this->ui.responses) {
            this->_delete_response();
            return true;
         } else if (auto* widget = this->ui.linkedTopics; object == widget) {
            this->_remove_linked_topic();
            return true;
         }
      }
   }
   return false;
}