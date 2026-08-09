#include "./DKAudioWidget.h"
#include <memory>
#include <QBoxLayout>
#if !defined(QT_PLUGIN)
   #include "editor/subsystems/audio/core.h"
   #include "editor/subsystems/audio/sound_category.h"
   #include "editor/subsystems/audio/sound_definition.h"
   #include "editor/subsystems/audio/sound_instance.h"
   #include "editor/subsystems/audio/utils/load_asset_as_sound_definition.h"
#endif

DKAudioWidget::DKAudioWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   {
      auto* button = this->subwidgets.play_pause = new QPushButton(tr("Play"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidget::playPause);
   }
   {
      auto* button = this->subwidgets.stop = new QPushButton(tr("Stop"));
      layout->addWidget(button);
      QObject::connect(button, &QPushButton::clicked, this, &DKAudioWidget::stop);
   }
   {
      auto* slider = this->subwidgets.seek = new QSlider(Qt::Orientation::Horizontal);
      layout->addWidget(slider);
      slider->setEnabled(this->allowSeeking());
      #if !defined(QT_PLUGIN)
         QObject::connect(slider, &QSlider::sliderPressed, this, [this]() {
            this->seek_poll_timer.stop();

            this->_state.dragged_while_playing = false;
            if (this->sound.instance)
               this->_state.dragged_while_playing = this->sound.instance->is_playing();

            const auto blocker = QSignalBlocker(this->sound.instance);
            this->sound.instance->pause();
         });
         QObject::connect(slider, &QSlider::sliderReleased, this, [this]() {
            const auto pos = (double)this->subwidgets.seek->value() / 1000;
            if (this->sound.instance) {
               const auto blocker = QSignalBlocker(this->sound.instance);
               this->sound.instance->play_from_s(pos);
               if (this->_state.dragged_while_playing) {
                  this->seek_poll_timer.start();
               } else {
                  this->sound.instance->pause();
               }
            }
         });
      #endif
   }

   this->seek_poll_timer.setInterval(50);
   this->seek_poll_timer.setSingleShot(false);
   #if !defined(QT_PLUGIN)
      QObject::connect(&this->seek_poll_timer, &QTimer::timeout, this, &DKAudioWidget::_update_seek_slider);
   #endif

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->subwidgets.play_pause);
   QWidget::setTabOrder(this->subwidgets.play_pause, this->subwidgets.stop);
   QWidget::setTabOrder(this->subwidgets.stop, this->subwidgets.seek);
}

#if !defined(QT_PLUGIN)
   QString DKAudioWidget::path() const {
      return this->sound.path;
   }
   void DKAudioWidget::setPath(QString v) {
      this->sound.path = v;
      this->_reload_sound_definition();
      this->_rebuild_sound_instance();
      this->_on_playback_state_changed();
   }
#endif

void DKAudioWidget::setAllowSeeking(bool v) {
   this->_properties.allow_seeking = v;
   this->subwidgets.seek->setEnabled(v);
}

bool DKAudioWidget::showSeekSlider() const noexcept {
   return !this->subwidgets.seek->isHidden();
}
void DKAudioWidget::setShowSeekSlider(bool v) {
   auto* widget = this->subwidgets.seek;
   if (!widget->isHidden() == v)
      return;
   widget->setVisible(v);
   #if !defined(QT_PLUGIN)
      this->_update_polling_sound_position();
   #endif
}

void DKAudioWidget::play() {
   #if !defined(QT_PLUGIN)
      if (!this->sound.instance)
         return;
      this->sound.instance->play();
      this->_on_playback_state_changed();
   #endif
}
void DKAudioWidget::playPause() {
   #if !defined(QT_PLUGIN)
      if (!this->sound.instance)
         return;
      if (this->sound.instance->is_playing())
         this->pause();
      else
         this->play();
   #endif
}
void DKAudioWidget::pause() {
   #if !defined(QT_PLUGIN)
      if (!this->sound.instance)
         return;
      this->sound.instance->pause();
   #endif
}
void DKAudioWidget::stop() {
   #if !defined(QT_PLUGIN)
      if (!this->sound.instance)
         return;
      this->sound.instance->stop();
   #endif
}

#if !defined(QT_PLUGIN)
   void DKAudioWidget::_reload_sound_definition() {
      this->sound.definition.reset();
      if (auto* p = this->sound.instance) {
         p->deleteLater();
         this->sound.instance = nullptr;
      }

      std::filesystem::path path = this->sound.path.toStdString();
      this->sound.definition = dovahkit::subsystems::audio::utils::load_asset_as_sound_definition(path);

      if (this->sound.definition) {
         auto  duration = this->sound.definition->estimated_length();
         auto* slider   = this->subwidgets.seek;
         slider->setRange(0, duration * 1000);
      }
   }
   void DKAudioWidget::_rebuild_sound_instance() {
      if (!this->sound.definition)
         return;
      auto* inst = this->sound.instance = new sound_instance(this, this->sound.definition);
      QObject::connect(inst, &sound_instance::paused,   this, &DKAudioWidget::_on_playback_state_changed);
      QObject::connect(inst, &sound_instance::stopped,  this, &DKAudioWidget::_on_playback_state_changed);
      QObject::connect(inst, &sound_instance::finished, this, &DKAudioWidget::_on_playback_state_changed);

      auto& core = dovahkit::subsystems::audio::core::get_or_create();
      core.set_sound_instance_category(*inst, dovahkit::subsystems::audio::sound_category::dialogue_preview);
   }
   void DKAudioWidget::_update_buttons() {
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
   void DKAudioWidget::_update_seek_slider() {
      auto* slider = this->subwidgets.seek;
      if (!this->sound.instance) {
         slider->setValue(0);
         this->seek_poll_timer.stop();
         return;
      }
      auto position = this->sound.instance->position();
      slider->setValue(std::chrono::duration_cast<std::chrono::milliseconds>(position).count());
   }

   bool DKAudioWidget::_should_poll_sound_position() const {
      if (!this->sound.instance)
         return false;
      if (!this->sound.instance->is_playing())
         return false;
      if (this->subwidgets.seek->isHidden())
         return false;
      return true;
   }
   void DKAudioWidget::_update_polling_sound_position() {
      bool is_polling  = this->seek_poll_timer.isActive();
      bool should_poll = this->_should_poll_sound_position();
      if (is_polling != should_poll) {
         if (should_poll)
            this->seek_poll_timer.start();
         else
            this->seek_poll_timer.stop();
      }
   }

   void DKAudioWidget::_on_playback_state_changed() {
      this->_update_buttons();
      this->_update_seek_slider();
      this->_update_polling_sound_position();
   }
#endif