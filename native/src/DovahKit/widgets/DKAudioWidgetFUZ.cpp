#include "./DKAudioWidgetFUZ.h"
#include <memory>
#include <QBoxLayout>
#include "editor/subsystems/audio/core.h"
#include "editor/subsystems/audio/sound_category.h"
#include "editor/subsystems/audio/sound_definitions/fuz.h"
#include "editor/subsystems/audio/sound_definitions/wav.h"
#include "editor/subsystems/audio/sound_definitions/xwma.h"
#include "editor/subsystems/audio/sound_instance.h"
#include "editor/subsystems/assets.h"
#include "dovah/files/bsa/bsa_archived_file.h"

namespace sound_definitions {
   using namespace dovahkit::subsystems::audio::sound_definitions;
}

DKAudioWidgetFUZ::DKAudioWidgetFUZ(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins({ 0, 0, 0, 0 });
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
   {
      auto* slider = this->subwidgets.seek = new QSlider(Qt::Orientation::Horizontal);
      layout->addWidget(slider);
      slider->setEnabled(false); // read-only
   }

   this->seek_poll_timer.setInterval(100);
   this->seek_poll_timer.setSingleShot(false);
   QObject::connect(&this->seek_poll_timer, &QTimer::timeout, this, &DKAudioWidgetFUZ::_update_seek_slider);
}

#if !defined(QT_PLUGIN)
   QString DKAudioWidgetFUZ::path() const {
      return this->sound.path;
   }
   void DKAudioWidgetFUZ::setPath(QString v) {
      this->sound.path = v;
      this->_reload_sound_definition();
      this->_rebuild_sound_instance();
      this->_on_playback_state_changed();
   }
#endif

void DKAudioWidgetFUZ::play() {
   if (!this->sound.instance)
      return;
   this->sound.instance->play();
   this->_on_playback_state_changed();
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

   auto ext    = path.extension();
   bool is_fuz = sound_definitions::fuz::data_is_likely_fuz(data->data(), data->size());
   bool is_xwm = false;
   bool is_wav = false;
   if (!is_fuz) {
      is_xwm = sound_definitions::xwma::data_is_likely_xwma(data->data(), data->size());
      if (!is_xwm)
         is_wav = sound_definitions::wav::data_is_likely_wav(data->data(), data->size());
   }

   if (!is_fuz && !is_xwm && !is_wav) {
      delete data;
      return;
   }

   std::unique_ptr<dovah::bsa_archived_file> data_ptr;
   data_ptr.reset(data);
   data = nullptr;
   if (is_fuz) {
      this->sound.definition = std::make_shared<sound_definitions::fuz>(std::move(data_ptr));
   } else if (is_xwm) {
      this->sound.definition = std::make_shared<sound_definitions::xwma>(std::move(data_ptr));
   } else if (is_wav) {
      this->sound.definition = std::make_shared<sound_definitions::wav>(std::move(data_ptr));
   }

   if (this->sound.definition) {
      auto  duration = this->sound.definition->estimated_length();
      auto* slider   = this->subwidgets.seek;
      slider->setRange(0, duration * 1000);
   }
}
void DKAudioWidgetFUZ::_rebuild_sound_instance() {
   if (!this->sound.definition)
      return;
   auto* inst = this->sound.instance = new sound_instance(this, this->sound.definition);
   QObject::connect(inst, &sound_instance::paused,   this, &DKAudioWidgetFUZ::_on_playback_state_changed);
   QObject::connect(inst, &sound_instance::stopped,  this, &DKAudioWidgetFUZ::_on_playback_state_changed);
   QObject::connect(inst, &sound_instance::finished, this, &DKAudioWidgetFUZ::_on_playback_state_changed);

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
void DKAudioWidgetFUZ::_update_seek_slider() {
   auto* slider = this->subwidgets.seek;
   if (!this->sound.instance) {
      slider->setValue(0);
      this->seek_poll_timer.stop();
      return;
   }
   auto position = this->sound.instance->position();
   slider->setValue(std::chrono::duration_cast<std::chrono::milliseconds>(position).count());
}
void DKAudioWidgetFUZ::_on_playback_state_changed() {
   this->_update_buttons();
   this->_update_seek_slider();

   bool is_polling = this->seek_poll_timer.isActive();
   bool is_playing = this->sound.instance->is_playing();
   if (is_playing != is_polling) {
      if (is_playing)
         this->seek_poll_timer.start();
      else
         this->seek_poll_timer.stop();
   }
}