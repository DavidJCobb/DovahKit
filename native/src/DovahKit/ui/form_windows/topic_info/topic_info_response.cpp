#include "./topic_info_response.h"
#include "dovah/files/tes_file_reading/file_loader.h" // for source filename
#include "dovah/form_stubs/helpers/get_dialogue_topic_quest.h"
#include "dovah/forms/Topic.h" // for topic prompt text
#include "dovah/utils/compute_voice_file_location.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "./TopicInfoResponseVoicesModel.h"
#include "ui/types/game_file_path.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/set_textarea_height_in_lines.h"
#include "ui/utils/typical_tableview_config.h"

FormSubdialogTopicInfoResponse::FormSubdialogTopicInfoResponse(QWidget* parent) {
   this->ui.setupUi(this);
   this->setWindowFlags(this->windowFlags() | Qt::WindowContextHelpButtonHint); // show "What's This?" button in title bar

   this->ui.animSpeaker->setAllowedFormType(dovah::form_type::idle);
   this->ui.animListener->setAllowedFormType(dovah::form_type::idle);

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

   this->ui.useSound->setAllowedFormType(dovah::form_type::sound_descriptor);

   {
      auto* model = this->_models.voices = new TopicInfoResponseVoicesModel(this);
      auto* view  = this->ui.voiceFilesTable;
      view->setModel(model);
      auto* sm = view->selectionModel();
      QObject::connect(sm, &QItemSelectionModel::selectionChanged, this, &FormSubdialogTopicInfoResponse::_on_voicetype_selection_changed);

      ui::typical_tableview_config(view);
      ui::set_tableview_column_flex(view, [model](DKHeaderView& header, const QFontMetrics& metrics) {
         {
            constexpr const auto column = TopicInfoResponseVoicesModel::Column::Voicetype;
            auto label_text  = model->headerData(column, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            auto label_width = metrics.boundingRect(label_text).width();
            auto value_width = metrics.boundingRect("FemaleUniqueMaven").width();
            header.setColumnFlex(column, 1, 1, std::max(label_width, value_width) * 1.5F + 4);
         }
         header.setColumnFlex(TopicInfoResponseVoicesModel::Column::FilePath, 2, 1);
      });
      this->_on_voicetype_selection_changed();
   }

   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);

   this->ensurePolished();
   ui::set_textarea_height_in_lines(*this->ui.text, 3);
   ui::set_textarea_height_in_lines(*this->ui.scriptNotes, 3);
   ui::set_textarea_height_in_lines(*this->ui.edits, 3);
}

void FormSubdialogTopicInfoResponse::importFrom(const loaded_form_type& src_form, const response_type& src) {
   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   this->ui.text->setPlainText(gls.convert_localized_string(src.text));
   this->ui.scriptNotes->setPlainText(QString::fromStdString(src.script_notes));
   this->ui.edits->setPlainText(QString::fromStdString(src.edits));

   this->ui.animSpeaker->setFormStub(src.idles.speaker.get_form_stub());
   this->ui.animListener->setFormStub(src.idles.listener.get_form_stub());
   this->ui.flagUseEmotionAnim->setChecked(src.flags & response_type::flag::use_emotion_animation);

   this->ui.emotionType->setCurrentIndex(this->ui.emotionType->findData((int)src.emotion.type));
   this->ui.emotionValue->setValue(src.emotion.value);

   this->ui.useSound->setFormStub(src.sound.get_form_stub());

   auto* info  = &src_form.stub;
   auto* topic = info->get_parent_form();
   auto* quest = topic ? dovah::form_stub_helpers::get_dialogue_topic_quest(*topic) : nullptr;
   {  // Topic Text and Prompt preview
      {
         auto prompt = gls.convert_localized_string(src_form.override_topic_text);
         if (!prompt.isEmpty())
            this->ui.promptPreview->setText(prompt);
      }
      if (topic && topic->form_type == dovah::form_type::topic) {
         auto loaded = topic->load().ptr_cast<dovah::loaded_forms::Topic>();
         if (loaded) {
            auto prompt = gls.convert_localized_string(loaded->text);
            if (!prompt.isEmpty())
               this->ui.topicTextPreview->setText(prompt);
         }
      }
   }
   {  // Filename
      std::string_view quest_editor_id;
      std::string_view topic_editor_id;
      if (topic) {
         topic_editor_id = topic->editorID;
         if (quest)
            quest_editor_id = quest->editorID;
      }

      std::string source_filename;
      {
         auto* file = src_form.stub.get_source_file_info(0);
         if (file && file->pointer) {
            source_filename = file->pointer->get_filename();
         }
      }
      this->_models.voices->setVoiceFileLocationInfo({
         .data_filename   = source_filename,
         .quest_editor_id = std::string(quest_editor_id),
         .topic_editor_id = std::string(topic_editor_id),
         .info_form_id    = src_form.stub.formID,
         .response_uid    = src.id,
      });

      auto filename = dovah::compute_voice_filename(
         quest_editor_id,
         topic_editor_id,
         info->formID,
         src.id
      );
      if (!filename.empty()) {
         this->ui.expectedVoiceFilename->setText(QString::fromStdString(filename));
      }
   }
}
void FormSubdialogTopicInfoResponse::exportTo(loaded_form_type& dst_form, response_type& dst) {
   auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
   gls.assign_localized_string(dst.text, this->ui.text->toPlainText());
   dst.script_notes = this->ui.scriptNotes->toPlainText().toStdString();
   dst.edits        = this->ui.edits->toPlainText().toStdString();

   dst.idles.speaker.set(dst_form, this->ui.animSpeaker->formStub());
   dst.idles.listener.set(dst_form, this->ui.animListener->formStub());
   if (this->ui.flagUseEmotionAnim->isChecked()) {
      dst.flags |= response_type::flag::use_emotion_animation;
   } else {
      dst.flags &= ~response_type::flag::use_emotion_animation;
   }

   dst.emotion.type  = (dovah::dialogue::emotion)this->ui.emotionType->currentData().toInt();
   dst.emotion.value = this->ui.emotionValue->value();

   dst.sound.set(dst_form, this->ui.useSound->formStub());
}

void FormSubdialogTopicInfoResponse::_on_voicetype_selection_changed() {
   auto* model  = this->_models.voices;
   auto* view   = this->ui.voiceFilesTable;
   auto* sm     = view->selectionModel();
   auto* player = this->ui.audioPlayer;

   auto sel = sm->selection();
   if (sel.empty()) {
      player->setEnabled(false);
      player->setPath({});
      return;
   }
   player->setEnabled(true);

   auto qmi = sel[0].topLeft();
   qmi = qmi.sibling(qmi.row(), TopicInfoResponseVoicesModel::Column::FilePath);
   auto path_str = this->_models.voices->data(qmi, Qt::DisplayRole).toString();
   auto path     = ui::types::game_file_path(path_str);
   path.scope_to_stem_folder("sound");
   player->setPath(path.to_string());
}