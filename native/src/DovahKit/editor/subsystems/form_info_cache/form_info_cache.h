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

namespace dovahkit::subsystems::form_info_cache {
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

         void cachedModelPathChanged(dovah::form_stub&, QString new_value);
         void cachedQuestFilterChanged(dovah::form_stub&, QString new_value);
         void cachedScriptsChanged(dovah::form_stub&);

      public:
         QString get_form_model_path(const dovah::form_stub&) const;
         QString get_quest_filter(const dovah::form_stub&) const;

         bool form_has_script_attached(const dovah::form_stub&, std::string_view scriptname) const;
   };
}