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
         class collision_layer;
         class enchantment;
         class faction;
         class head_part;
         class magic_effect;
         class music_track;
         class quest;
         class package;
         class topic;
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

         void cachedModelPathChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedQuestFilterChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedScriptsChanged(dovah::form_stub&);
         void cachedSharedInfoTopicChanged(dovah::form_stub&, bool became_sharedinfo_topic);

         // I wanted to define these using FOR_EACH_CACHED_FORM_TYPE, to ensure consistency 
         // across source files. Unfortunately, Qt's MOC tool is highly unpleasant and chokes 
         // on any attempt to do so.
         //
         // We need MOC to see these functions, since it's what creates the code that powers 
         // them.
         void cachedActorBaseChanged(dovah::form_stub&);
         void cachedCollisionLayerChanged(dovah::form_stub&);
         void cachedEnchantmentChanged(dovah::form_stub&);
         void cachedFactionChanged(dovah::form_stub&);
         void cachedHeadPartChanged(dovah::form_stub&);
         void cachedMagicEffectChanged(dovah::form_stub&);
         void cachedMusicTrackChanged(dovah::form_stub&);
         void cachedPackageChanged(dovah::form_stub&);
         void cachedQuestChanged(dovah::form_stub&);
         void cachedTopicChanged(dovah::form_stub&);
         void cachedVoicetypeChanged(dovah::form_stub&);

      public:
         QString get_form_model_path(const dovah::form_stub&) const;
         QString get_quest_filter(const dovah::form_stub&) const;

         #ifndef Q_MOC_RUN // again, MOC sucks
            #pragma region const cached_data::by_form::TYPE* get_TYPE_info(const dovah::form_stub&) const
               #include "./macros/FOR_EACH_CACHED_FORM_TYPE.define.h"
               #define X(_type, ...) const cached_data::by_form::_type* get_##_type##_info(const dovah::form_stub&) const;
               FOR_EACH_CACHED_FORM_TYPE(X);
               #undef X
               #include "./macros/FOR_EACH_CACHED_FORM_TYPE.undef.h"
            #pragma endregion
         #endif

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