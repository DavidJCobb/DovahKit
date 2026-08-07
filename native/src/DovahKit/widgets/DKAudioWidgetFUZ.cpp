#include "./DKAudioWidgetFUZ.h"
#include <memory>
#include <QBoxLayout>
#include "editor/subsystems/audio/core.h"
#include "editor/subsystems/audio/sound_category.h"
#include "editor/subsystems/audio/sound_definitions/fuz.h"
#include "editor/subsystems/audio/sound_instance.h"
#include "editor/subsystems/assets.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/fuz/file_info.h"
#include "xaudio2/xwma_file_info.h"

DKAudioWidgetFUZ::DKAudioWidgetFUZ(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   {
      auto* button = this->subwidgets.play_pause = new QPushButton(tr("Play"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidgetFUZ::playPause);
   }
   {
      auto* button = this->subwidgets.stop = new QPushButton(tr("Stop"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidgetFUZ::stop);
   }
}

#if !defined(QT_PLUGIN)
   QString DKAudioWidgetFUZ::path() const {
      return this->sound.path;
   }
   void DKAudioWidgetFUZ::setPath(QString v) {
      this->sound.path = v;
      this->_reload_sound_definition();
      this->_rebuild_sound_instance();
      this->_update_buttons();
   }
#endif

void DKAudioWidgetFUZ::play() {
   if (!this->sound.instance)
      return;
   this->sound.instance->play();
   this->_update_buttons();
}
void DKAudioWidgetFUZ::playPause() {
   if (!this->sound.instance)
      return;
   if (this->sound.instance->is_playing())
      this->pause();
   else
      this->play();
}
void DKAudioWidgetFUZ::pause() {
   if (!this->sound.instance)
      return;
   this->sound.instance->pause();
}
void DKAudioWidgetFUZ::stop() {
   if (!this->sound.instance)
      return;
   this->sound.instance->stop();
}

void DKAudioWidgetFUZ::_reload_sound_definition() {
   this->sound.definition.reset();
   if (auto* p = this->sound.instance) {
      p->deleteLater();
      this->sound.instance = nullptr;
   }

   std::filesystem::path path = this->sound.path.toStdString();
   auto* data = dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path, true);
   if (!data) {
      return;
   }

   // Validity checks:
   {
      dovah::fuz::file_info info(data->data(), data->size());
      if (!info.buffer.data) {
         delete data;
         return;
      }
      auto* riff_data = (const uint8_t*)data->data() + dovah::fuz::header_size + info.buffer.size;
      auto  riff_size = data->size() - (dovah::fuz::header_size + info.buffer.size);
      dovahkit::xaudio2::xwma_file_info audio_info{ riff_data, riff_size };
      if (!audio_info.valid()) {
         delete data;
         return;
      }
   }

   std::unique_ptr<dovah::bsa_archived_file> data_ptr;
   data_ptr.reset(data);
   data = nullptr;
   this->sound.definition = std::make_shared<dovahkit::subsystems::audio::sound_definitions::fuz>(std::move(data_ptr));
}
void DKAudioWidgetFUZ::_rebuild_sound_instance() {
   if (!this->sound.definition)
      return;
   auto* inst = this->sound.instance = new sound_instance(this, this->sound.definition);
   QObject::connect(inst, &sound_instance::paused,   this, &DKAudioWidgetFUZ::_update_buttons);
   QObject::connect(inst, &sound_instance::stopped,  this, &DKAudioWidgetFUZ::_update_buttons);
   QObject::connect(inst, &sound_instance::finished, this, &DKAudioWidgetFUZ::_update_buttons);

   auto& core = dovahkit::subsystems::audio::core::get_or_create();
   core.set_sound_instance_category(*inst, dovahkit::subsystems::audio::sound_category::dialogue_preview);
}
void DKAudioWidgetFUZ::_update_buttons() {
   if (!this->sound.instance) {
      this->subwidgets.play_pause->setText(tr("Play"));
      this->subwidgets.play_pause->setEnabled(false);
      this->subwidgets.stop->setEnabled(false);
      return;
   }
   this->subwidgets.play_pause->setEnabled(true);
   if (this->sound.instance->is_playing()) {
      this->subwidgets.play_pause->setText(tr("Pause"));
      this->subwidgets.stop->setEnabled(true);
   } else {
      this->subwidgets.play_pause->setText(tr("Play"));
      if (this->sound.instance->is_at_start()) {
         this->subwidgets.stop->setEnabled(false);
      } else {
         this->subwidgets.stop->setEnabled(true);
      }
   }
}