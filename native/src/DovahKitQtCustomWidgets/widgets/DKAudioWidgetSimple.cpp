#include "./DKAudioWidgetSimple.h"
#include <memory>
#include <QBoxLayout>
#include "editor/subsystems/assets.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/fuz/file_info.h"
#include "xaudio2/xwma_file_info.h"

DKAudioWidgetSimple::DKAudioWidgetSimple(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   {
      auto* button = this->subwidgets.play_pause = new QPushButton(tr("Play"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidgetSimple::playPause);
   }
   {
      auto* button = this->subwidgets.stop = new QPushButton(tr("Stop"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidgetSimple::stop);
   }
}

#if !defined(QT_PLUGIN)
   void DKAudioWidgetSimple::makeAudioCore() {
      if (this->sound.instance) {
         this->sound.instance.reset();
      }
      this->audio_core = std::make_shared<audio_core_type>();
      if (!this->sound.path.isEmpty()) {
         this->setPath(this->sound.path);
      }
   }
   void DKAudioWidgetSimple::setAudioCore(const std::shared_ptr<audio_core_type>& core) {
      if (core == this->audio_core)
         return;
      if (this->sound.instance) {
         this->sound.instance.reset();
      }
      this->audio_core = core;
      if (!this->sound.path.isEmpty()) {
         this->setPath(this->sound.path);
      }
   }

   QString DKAudioWidgetSimple::path() const {
      return this->sound.path;
   }
   void DKAudioWidgetSimple::setPath(QString v) {
      this->sound.path = v;
      this->_reload_sound_definition();
      this->_rebuild_sound_instance();
      this->_update_buttons();
   }
#endif

void DKAudioWidgetSimple::play() {
   if (!this->sound.instance)
      return;
   this->sound.instance->play();
   this->_update_buttons();
}
void DKAudioWidgetSimple::playPause() {
   if (!this->sound.instance)
      return;
   if (this->sound.instance->is_playing())
      this->pause();
   else
      this->play();
}
void DKAudioWidgetSimple::pause() {
   if (!this->sound.instance)
      return;
   this->sound.instance->pause();
}
void DKAudioWidgetSimple::stop() {
   if (!this->sound.instance)
      return;
   this->sound.instance->stop();
}

void DKAudioWidgetSimple::_reload_sound_definition() {
   this->sound.definition.reset();
   this->sound.instance.reset();

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
   this->sound.definition = std::make_shared<sound_definition>(std::move(data_ptr));
}
void DKAudioWidgetSimple::_rebuild_sound_instance() {
   if (!this->audio_core)
      return;
   if (!this->sound.definition)
      return;
   this->sound.instance = std::make_unique<sound_instance>(*this->audio_core, this->sound.definition);

   auto* inst = this->sound.instance.get();
   QObject::connect(inst, &sound_instance::paused,   this, &DKAudioWidgetSimple::_update_buttons);
   QObject::connect(inst, &sound_instance::stopped,  this, &DKAudioWidgetSimple::_update_buttons);
   QObject::connect(inst, &sound_instance::finished, this, &DKAudioWidgetSimple::_update_buttons);
}
void DKAudioWidgetSimple::_update_buttons() {
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