#include "./topic_info_response.h"
#include <array>
#include "editor/core.h"

namespace {
   // Constexpr so we can enable these as they're implemented.
   constexpr const bool enable_loading_custom_wav  = false;
   constexpr const bool enable_browsing_voicetypes = false;
}

FormSubdialogTopicInfoResponse::FormSubdialogTopicInfoResponse(QWidget* parent) {
   this->ui.setupUi(this);

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

   if constexpr (!enable_loading_custom_wav) {
      this->ui.buttonLoadWavForVoicetype->setVisible(false);
   }
   if constexpr (!enable_browsing_voicetypes) {
      this->ui.voiceFilesTable->setVisible(false);
      this->ui.buttonViewVoicetypeNPCs->setVisible(false);
   }
}

void FormSubdialogTopicInfoResponse::importFrom(const loaded_form_type& src_form, const response_type& src) {
   auto& editor = DovahKitCore::get();
   this->ui.text->setPlainText(editor.convert_localized_string(src.text));
   this->ui.scriptNotes->setPlainText(editor.convert_localized_string(src.script_notes));
   this->ui.edits->setPlainText(editor.convert_localized_string(src.edits));

   this->ui.animSpeaker->setFormStub(src.idles.speaker.get_form_stub());
   this->ui.animListener->setFormStub(src.idles.listener.get_form_stub());
   this->ui.flagUseEmotionAnim->setChecked(src.flags & response_type::flag::use_emotion_animation);

   this->ui.emotionType->setCurrentIndex(this->ui.emotionType->findData((int)src.emotion.type));
   this->ui.emotionValue->setValue(src.emotion.value);

   this->ui.useSound->setFormStub(src.sound.get_form_stub());
}
void FormSubdialogTopicInfoResponse::exportTo(loaded_form_type& dst_form, response_type& dst) {
   auto& editor = DovahKitCore::get();
   editor.assign_localized_string(dst.text,         this->ui.text->toPlainText());
   editor.assign_localized_string(dst.script_notes, this->ui.scriptNotes->toPlainText());
   editor.assign_localized_string(dst.edits,        this->ui.edits->toPlainText());

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