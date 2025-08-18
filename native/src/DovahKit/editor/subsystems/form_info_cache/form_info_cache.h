#pragma once
#include <functional>
#include <vector>
#include <QObject>
#include <QString>
#include "helpers/singleton_ex.h"
#include "dovah/form_types.h"

namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes_file_reading {
      class record;
   }
   class form_stub;
}
namespace dovahkit::subsystems::form_info_cache {
   namespace cached_data {
      namespace by_form {
         class actor_base;
         class faction;
         class head_part;
         class magic_effect;
         class music_track;
         class package;
         class voicetype;
      }
   }
   struct entire_cache;
}
namespace dovahkit::subsystems::papyrus {
   class known_script;
}

namespace dovahkit::subsystems::form_info_cache {
   enum class script_attach_state {
      not_present,
      attached,
      removed,
   };

   class core;
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      private:
         template<dovah::form_type FormType>
         void _skim_record(dovah::form_stub&, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc);
         
         template<dovah::form_type FormType>
         static void _static_skim_record(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
            core::get()._skim_record<FormType>(stub, record, intfc);
         }

      protected:
         entire_cache* _cache = nullptr; // use a pointer for the PImpl idiom, to hopefully avoid recompiling more than is necessary if we add/edit a cached data type

      protected slots:
         void buildAllData();
         void clear();

      signals:
         void cachedDataBuilt(); // emitted when all data is built
         void cachedDataCleared(); // emitted when all data is cleared

         void cachedActorBaseChanged(dovah::form_stub&);
         void cachedFactionChanged(dovah::form_stub&);
         void cachedHeadPartChanged(dovah::form_stub&);
         void cachedMagicEffectChanged(dovah::form_stub&);
         void cachedModelPathChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedMusicTrackChanged(dovah::form_stub&);
         void cachedQuestFilterChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedPackageChanged(dovah::form_stub&);
         void cachedScriptsChanged(dovah::form_stub&);
         void cachedVoicetypeChanged(dovah::form_stub&);
         void cachedSharedInfoTopicChanged(dovah::form_stub&, bool became_sharedinfo_topic);

      public:
         QString get_form_model_path(const dovah::form_stub&) const;
         QString get_quest_filter(const dovah::form_stub&) const;
         const cached_data::by_form::actor_base*   get_actor_base_info(const dovah::form_stub&) const;
         const cached_data::by_form::faction*      get_faction_info(const dovah::form_stub&) const;
         const cached_data::by_form::head_part*    get_head_part_info(const dovah::form_stub&) const;
         const cached_data::by_form::magic_effect* get_magic_effect_info(const dovah::form_stub&) const;
         const cached_data::by_form::music_track*  get_music_track_info(const dovah::form_stub&) const;
         const cached_data::by_form::package*      get_package_info(const dovah::form_stub&) const;
         const cached_data::by_form::voicetype*    get_voicetype_info(const dovah::form_stub&) const;

         script_attach_state form_script_attachment(const dovah::form_stub&, std::string_view scriptname) const;
         bool quest_has_alias_with_script(const dovah::form_stub&, std::string_view scriptname) const;
         //
         std::vector<const subsystems::papyrus::known_script*> get_scripts_attached_to_form(const dovah::form_stub&) const;
         std::vector<const subsystems::papyrus::known_script*> get_scripts_attached_to_quest_aliases(const dovah::form_stub& quest) const;

         bool topic_is_sharedinfo_topic(const dovah::form_stub& topic) const;

         void for_all_form_model_paths(std::function<void(const dovah::form_stub&, QString)> functor);
         void for_all_quest_filters(std::function<void(QString)> functor);
         void for_all_head_parts(std::function<void(const cached_data::by_form::head_part&)> functor);
   };
}