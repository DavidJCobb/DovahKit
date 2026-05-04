#pragma once
#include <memory>
#include <QPushButton>
#include <QWidget>
#include "xaudio2/core_interface.h"
#include "xaudio2/simple_fuz_sound.h"

class DKAudioWidgetSimple : public QWidget {
   Q_OBJECT;
   public:
      using audio_core_type  = dovahkit::xaudio2::core_interface;
      using sound_definition = dovahkit::xaudio2::simple_fuz_sound_definition;
      using sound_instance   = dovahkit::xaudio2::simple_fuz_sound_instance;

   public:
      DKAudioWidgetSimple(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         const std::shared_ptr<audio_core_type>& audioCore() const noexcept { return this->audio_core; }
         void makeAudioCore();
         void setAudioCore(const std::shared_ptr<audio_core_type>&);

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
         std::shared_ptr<audio_core_type> audio_core;
         struct {
            std::shared_ptr<sound_definition> definition;
            std::unique_ptr<sound_instance>   instance;
            QString path;
         } sound;
      #endif
      struct {
         QPushButton* play_pause = nullptr;
         QPushButton* stop       = nullptr;
      } subwidgets;

      void _reload_sound_definition();
      void _rebuild_sound_instance();
      void _update_buttons();
};