#include "game_inis.h"
#include <QDir>
#include <QStandardPaths>
#include "../../helpers/qt/ini.h"
#include "../../dovah/data/ini_settings.h"
#include "../core.h"

namespace {
   cobb::qt::ini::File* construct_ini(const dovah::game_ini::file_definition& defs) {
      using namespace cobb::qt::ini;
      //
      auto* instance = new File({ .path = "" });
      //
      for (auto& c : defs.sections) {
         QString category = QString::fromUtf8(c.name.c_str());
         for (auto& s : c.settings) {
            switch (s.type()) {
               using _ = dovah::game_ini::setting_type;
               case _::boolean:
                  new Setting(*instance, category, s.name, s.default_value.b);
                  break;
               case _::float32:
                  new Setting(*instance, category, s.name, s.default_value.f);
                  break;
               case _::integer:
                  new Setting(*instance, category, s.name, s.default_value.i);
                  break;
               case _::integer_unsigned:
                  new Setting(*instance, category, s.name, s.default_value.u);
                  break;
               case _::string:
                  new Setting(*instance, category, s.name, s.default_value.s);
                  break;
            }
         }
      }
      //
      return instance;
   }

   QString get_ini_path(dovah::game g) {
      QDir docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      switch (g) {
         using _ = dovah::game;
         case _::skyrim_special:
            return docs.absoluteFilePath("My Games/Skyrim Special Edition/");
         case _::skyrim_classic:
         default:
            return docs.absoluteFilePath("My Games/Skyrim/");
      }
   }
}

namespace editor::game_inis {
   extern cobb::qt::ini::File& get_skyrim() {
      static cobb::qt::ini::File* instance = nullptr;
      if (instance)
         return *instance;
      //
      instance = construct_ini(dovah::game_ini::files::skyrim);
      return *instance;
   }

   extern cobb::qt::ini::File& get_skyrim_prefs() {
      static cobb::qt::ini::File* instance = nullptr;
      if (instance)
         return *instance;
      //
      instance = construct_ini(dovah::game_ini::files::skyrim_prefs);
      return *instance;
   }

   extern void load_inis(dovah::game g) {
      QString base = get_ini_path(g);
      //
      {
         auto& ini = get_skyrim();
         ini.setPath(base + "/Skyrim.ini");
         for (auto* s : ini.allSettings()) {
            const auto blocker = QSignalBlocker(s);
            s->discardPendingValue();
            s->setCurrentValue(QVariant());
         }
         ini.load();
      }
      {
         auto& ini = get_skyrim_prefs();
         ini.setPath(base + "/SkyrimPrefs.ini");
         for (auto* s : ini.allSettings()) {
            const auto blocker = QSignalBlocker(s);
            s->discardPendingValue();
            s->setCurrentValue(QVariant());
         }
         ini.load();
      }
   }
}