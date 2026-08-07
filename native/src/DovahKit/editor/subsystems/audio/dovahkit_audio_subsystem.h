#pragma once
#include <array>
#include <vector>
#include <QObject>
#include "helpers/singleton_ex.h"
#include "./impl/engine_and_thread.h"
namespace dovahkit {
   namespace subsystems::audio {
      enum class sound_category;
      class sound_instance;
   }
}
class IXAudio2SubmixVoice;

namespace dovahkit::subsystems::audio {
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         const IXAudio2SubmixVoice* get_sound_category_submix(sound_category) const;
         IXAudio2SubmixVoice* get_sound_category_submix(sound_category);

         impl::engine_and_thread& get_engine() {
            return this->_engine_and_thread;
         }

         void set_sound_instance_category(sound_instance&, sound_category);
         
         void set_category_volume(sound_category, float);
         void set_master_volume(float);

      protected:
         impl::engine_and_thread _engine_and_thread;
         union {
            std::array<IXAudio2SubmixVoice*, 2> list = { 0 };
            struct {
               IXAudio2SubmixVoice* uncategorized;
               IXAudio2SubmixVoice* dialogue_preview;
            };
         } _sound_category_submixes;

         IXAudio2SubmixVoice* _make_sound_category_submix();

      signals:
         void onBeforeTeardown();
   };
};