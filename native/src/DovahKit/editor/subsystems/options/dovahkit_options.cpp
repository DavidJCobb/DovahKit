#include "./dovahkit_options.h"
#include <cstdio>
#include <fstream>
#include <istream>
#include <ostream>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QString>
#include <QTimer>
#include "editor/ini/main.h"

namespace dovahkit::subsystems::options {
   void option_collection::done_constructing() {
      core::get_or_create()._register_collection({}, *this);
   }

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
      auto dir  = QDir(QDir(path).absoluteFilePath("userdata/"));
      if (!dir.exists()) {
         dir.setPath(QDir::current().absoluteFilePath("userdata/")); // During debugging, the program's path is at ./x64/ConfigurationName/ and the current working directory is at ./
      }
      return dir.path() + "/";
   }
   QString core::get_base_options_path() {
      return get_userdata_path() + "options/";
   }
   QString core::get_main_ini_path() {
      return get_base_options_path() + "main.ini";
   }

   QString core::get_user_script_path() {
      return get_userdata_path() + "scripts/";
   }
   QString core::get_user_script_package_path() {
      return get_userdata_path() + "script-packages/";
   }

   void core::reload() {
      auto path = get_main_ini_path().toStdWString();
      std::ifstream stream(path, std::ios::in);
      if (stream.good()) {
         ::dovahkit::ini::main::file_data.load(stream);
      }

      for (auto* c : this->_collections)
         c->reload();
   }

   void core::save() {
      // QSaveFile::handle doesn't return a usable value, so we have to write to memory
      // TODO: Just write my own file handling routine for it so we at least get *some* 
      //       benefit from using streams
      std::string dst;

      {
         auto path = get_main_ini_path().toStdWString();
         std::ifstream src_stream(path, std::ios::in);
         if (src_stream.good()) {
            ::dovahkit::ini::main::file_data.save(dst, src_stream);
         } else {
            ::dovahkit::ini::main::file_data.save(dst);
         }
      }

      QSaveFile dst_file(get_main_ini_path());
      dst_file.setDirectWriteFallback(true);
      dst_file.open(QIODevice::WriteOnly);
      dst_file.write(dst.data(), dst.size());
      dst_file.commit();

      for (auto* c : this->_collections)
         c->save();
   }

   void core::_register_collection(cobb::passkey<core, option_collection>, option_collection& c) {
      this->_collections.push_back(&c);

      // We'll have already done our initial load, so we need to manually 
      // tell the newly-discovered collection to load. We need a single-shot 
      // timer because `c` registers itself during a superclass constructor, 
      // so we can't call vfuncs on it until it actually is done constructing.
      c.reload();
   }
}