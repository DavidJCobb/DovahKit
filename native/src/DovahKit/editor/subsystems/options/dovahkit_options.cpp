#include "./dovahkit_options.h"
#include <cstdio>
#include <fstream>
#include <istream>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include "editor/ini/main.h"

namespace dovahkit::subsystems::options {
   core::core() {
      //
      // NOTE: If at any point this constructor accesses DovahKitCore, then DovahKitCore's own 
      // constructor needs to be modified: we'll need to ensure that this subsystem is constructed 
      // in DovahKitCore's single-shot timer, to prevent cyclical dependencies between constructors.
      //
      ::dovahkit::ini::main::file_data.add_global_setting_change_callback(&_main_ini_change_callback);
      this->reload();
   }
   core::~core() {
      ::dovahkit::ini::main::file_data.remove_global_setting_change_callback(&_main_ini_change_callback);
   }

   /*static*/ void core::_main_ini_change_callback(cobb::ini::setting& s, cobb::ini::value_variant prior, cobb::ini::value_variant after) {
      emit core::get().mainIniSettingChanged(s, prior, after);
   }

   QString core::get_userdata_path() {
      //
      // TODO: We really should look into storing userdata in %APPDATA%. However, if we do, 
      // then things like Dovahscript package paths will need to be updated to do so as well.
      //
      auto path = QCoreApplication::applicationDirPath();
      return QDir(path).absoluteFilePath("userdata/");
   }

   void core::reload() {
      auto path = get_userdata_path() + "main.ini";

      QFile main_ini(get_userdata_path() + "main.ini");
      main_ini.open(QIODevice::ReadOnly);
      if (main_ini.isReadable()) {
         auto stream = std::ifstream(_fdopen(main_ini.handle(), "r"));
         ::dovahkit::ini::main::file_data.load(stream);
      }
   }
}