#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <QObject>
#include <QString>
#include "helpers/singleton_ex.h"

namespace dovahkit::subsystems::papyrus {
   class known_script;

   class core;
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      public:
         core();
         ~core();

      protected:
         std::unordered_map<std::string, known_script*> _known_scripts_by_name; // keys are forced to ASCII-lowercase

         static std::string _normalize_scriptname(std::string_view);
         static std::string _normalize_scriptname(QString);

         known_script* _scan_pex(const std::string& filename_sans_ext, const uint8_t* data, const size_t size, bool is_loose);

         void _update_superclass_of(known_script&);

         known_script* _lookup_known_script(std::string_view scriptname) const {
            return const_cast<known_script*>(std::as_const(*this).lookup_known_script(scriptname));
         }

      public:
         const known_script* lookup_known_script(std::string_view scriptname) const;
         const known_script* lookup_known_script(QString scriptname) const;

         void index_all_pex_files();

      signals:
         void pexIndexingComplete();

         // Signals emitted after initial PEX indexing is complete:
         void knownScriptDiscovered(const known_script&);
         void knownScriptChanged(const known_script&);
         void knownScriptAboutToBeForgotten(const known_script&);
         void knownScriptForgotten(const QString scriptname);
   };
}
