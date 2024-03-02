#pragma once
#include <mutex>
#include <QHash>
#include <QObject>
#include <QString>
#include "helpers/singleton_ex.h"

#include "dovah/form_types.h"

#include "./cache_map_collection.h"

namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace tes_file_reading {
      class record;
   }
   class form_stub;
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

      private:
         template<dovah::form_type::type FormType>
         void _skim_record(dovah::form_stub&, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc);
         
         template<dovah::form_type::type FormType>
         static void _static_skim_record(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
            core::get()._skim_record<FormType>(stub, record, intfc);
         }

      protected:
         cache_map_collection _cache;

      protected slots:
         void buildAllData();
         void clear();

      signals:
         void cachedDataBuilt(); // emitted when all data is built
         void cachedDataCleared(); // emitted when all data is cleared

         void cachedModelPathChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedQuestFilterChanged(dovah::form_stub&, QString old_value, QString new_value);
         void cachedScriptsChanged(dovah::form_stub&);

      public:
         QString get_form_model_path(const dovah::form_stub&) const;
         QString get_quest_filter(const dovah::form_stub&) const;

         script_attach_state form_script_attachment(const dovah::form_stub&, std::string_view scriptname) const;
         bool quest_has_alias_with_script(const dovah::form_stub&, std::string_view scriptname) const;

         std::vector<const subsystems::papyrus::known_script*> get_scripts_attached_to_form(const dovah::form_stub&) const;
         std::vector<const subsystems::papyrus::known_script*> get_scripts_attached_to_quest_aliases(const dovah::form_stub& quest) const;

         template<typename Functor> requires std::is_invocable_v<Functor, dovah::form_stub&, QString>
         void for_all_form_model_paths(Functor&& functor) {
            auto& list = this->_cache.model_paths;
            for (auto it = list.constKeyValueBegin(); it != list.constKeyValueEnd(); ++it)
               functor(*(it->first), it->second);
         }
         
         template<typename Functor> requires std::is_invocable_v<Functor, QString>
         void for_all_quest_filters(Functor&& functor) {
            for (auto path : this->_cache.quest_filters)
               functor(path);
         }
   };
}