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

class DKAudioWidgetFUZ : public QWidget {
   Q_OBJECT;
   public:
      using sound_definition = dovahkit::subsystems::audio::sound_definition;
      using sound_instance   = dovahkit::subsystems::audio::sound_instance;

   public:
      DKAudioWidgetFUZ(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         QString path() const;
         void setPath(QString);
      #endif

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
         QPushButton* play_pause = nullptr;
         QPushButton* stop       = nullptr;
         QSlider*     seek       = nullptr;
      } subwidgets;
      QTimer seek_poll_timer;

      void _reload_sound_definition();
      void _rebuild_sound_instance();
      void _update_buttons();
      void _update_seek_slider();

      void _on_playback_state_changed();
};