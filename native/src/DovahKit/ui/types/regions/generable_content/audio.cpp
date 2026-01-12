#include "./audio.h"
#include "dovah/forms/Region.h"
#include "dovah/forms/Sound.h"

namespace ui::types::regions::generable_content {
   void audio::clear() {
      this->ambient_sounds.clear();
      this->music_type = nullptr;
   }
   void audio::import_data(const dovah::loaded_forms::Region& src_form) {
      this->clear();

      for (auto& src_coll : src_form.generable_content) {
         auto* src_cast = src_coll.as<backend_collection_type>();
         if (!src_cast)
            continue;
         this->import_data(*src_cast);
      }
   }
   void audio::import_data(const backend_collection_type& src_coll) {
      this->music_type = src_coll.music.get_form_stub();
      if (this->music_type && this->music_type->form_type != dovah::form_type::music_type)
         this->music_type = nullptr;

      auto& src_list = src_coll.ambient_sounds;
      this->ambient_sounds.reserve(this->ambient_sounds.size() + src_list.size());
      for (size_t i = 0; i < src_list.size(); ++i) {
         auto& src_item = src_list[i];
         auto* sound    = src_item.form.get_form_stub();
         if (!sound)
            continue;
         if (sound->form_type != dovah::form_type::sound_descriptor) {
            if (sound->form_type != dovah::form_type::sound)
               continue;
            auto loaded = sound->load().ptr_cast<dovah::loaded_forms::Sound>();
            if (!loaded)
               continue;
            sound = loaded->descriptor.get_form_stub();
            if (sound->form_type != dovah::form_type::sound_descriptor)
               continue;
         }
         auto& dst_item = this->ambient_sounds.emplace_back();
         dst_item.sound  = sound;
         dst_item.chance = src_item.chance;
         dst_item.weather.pleasant = src_item.flags & backend_collection_type::ambient_sound::flag::weather_pleasant;
         dst_item.weather.cloudy   = src_item.flags & backend_collection_type::ambient_sound::flag::weather_cloudy;
         dst_item.weather.rainy    = src_item.flags & backend_collection_type::ambient_sound::flag::weather_rainy;
         dst_item.weather.snowy    = src_item.flags & backend_collection_type::ambient_sound::flag::weather_snowy;
      }
   }
   void audio::export_data(dovah::loaded_forms::Region& dst_form, backend_collection_type& dst_coll) const {
      dst_coll.clear(dst_form);
      dst_coll.music.set(dst_form, this->music_type);
      for (auto& src_item : this->ambient_sounds) {
         if (!src_item.sound)
            continue;
         auto& dst_item = dst_coll.ambient_sounds.emplace_back();
         dst_item.form.set(dst_form, src_item.sound);
         dst_item.chance = src_item.chance;
         dst_item.flags = 0;
         if (src_item.weather.pleasant)
            dst_item.flags |= backend_collection_type::ambient_sound::flag::weather_pleasant;
         if (src_item.weather.cloudy)
            dst_item.flags |= backend_collection_type::ambient_sound::flag::weather_cloudy;
         if (src_item.weather.rainy)
            dst_item.flags |= backend_collection_type::ambient_sound::flag::weather_rainy;
         if (src_item.weather.snowy)
            dst_item.flags |= backend_collection_type::ambient_sound::flag::weather_snowy;
      }
   }
}