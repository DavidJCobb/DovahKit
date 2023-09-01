#pragma once
#include <vector>
#include <QObject>
#include <QString>
#include "helpers/ini/types.h"
#include "helpers/passkey.h"
#include "helpers/singleton_ex.h"

namespace cobb::ini {
   class setting;
}

namespace dovahkit::subsystems::options {
   class core;

   class option_collection {
      protected:
         // DO NOT forget to call this at the end of your subclass constructor.
         void done_constructing();

      public:
         virtual void reload() = 0;
         virtual void save() = 0;
   };

   //
   // Subsystem for accessing game assets.
   //
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

         static void _main_ini_change_callback(cobb::ini::setting&, cobb::ini::value_variant prior, cobb::ini::value_variant after);

         std::vector<option_collection*> _collections;

      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

      public:
         QString get_userdata_path();

      signals:
         void mainIniSettingChanged(cobb::ini::setting&, cobb::ini::value_variant prior, cobb::ini::value_variant after);

      public slots:
         void reload();
         void save();

      public:
         void _register_collection(cobb::passkey<core, option_collection>, option_collection&);
   };
};