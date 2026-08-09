#pragma once
#include <memory>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QWidget>
namespace dovahkit::subsystems::audio {
   class sound_definition;
   class sound_instance;
}

class DKAudioWidget : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool allowSeeking   READ allowSeeking   WRITE setAllowSeeking   DESIGNABLE true);
   Q_PROPERTY(bool showSeekSlider READ showSeekSlider WRITE setShowSeekSlider DESIGNABLE true);
   public:
      using sound_definition = dovahkit::subsystems::audio::sound_definition;
      using sound_instance   = dovahkit::subsystems::audio::sound_instance;

   public:
      DKAudioWidget(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         QString path() const;
         void setPath(QString);
      #endif

      constexpr bool allowSeeking() const noexcept { return this->_properties.allow_seeking; }
      void setAllowSeeking(bool);

      bool showSeekSlider() const noexcept;
      void setShowSeekSlider(bool);

   public slots:
      void play();
      void playPause();
      void pause();
      void stop();
      
   protected:
      #if !defined(QT_PLUGIN)
         struct {
            std::shared_ptr<sound_definition> definition;
            sound_instance* instance = nullptr; // QObject pointer
            QString path;
         } sound;
      #endif
      struct {
         bool allow_seeking = true;
      } _properties;
      struct {
         bool dragged_while_playing = false;
      } _state;
      struct {
         QPushButton* play_pause = nullptr;
         QPushButton* stop       = nullptr;
         QSlider*     seek       = nullptr;
      } subwidgets;
      QTimer seek_poll_timer;

      #if !defined(QT_PLUGIN)
         void _reload_sound_definition();
         void _rebuild_sound_instance();
         void _update_buttons();
         void _update_seek_slider();

         bool _should_poll_sound_position() const;
         void _update_polling_sound_position();

         void _on_playback_state_changed();
      #endif
};