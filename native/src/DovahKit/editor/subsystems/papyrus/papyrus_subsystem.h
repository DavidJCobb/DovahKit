#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <QFileSystemWatcher>
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
         struct {
            bool    folder_exists = false;
            QString folder_path;
            QFileSystemWatcher watcher;
         } _loose_pex;

         void _teardown();

         static std::string _normalize_scriptname(std::string_view);
         static std::string _normalize_scriptname(QString);

         known_script* _scan_pex(const std::string& filename_sans_ext, const uint8_t* data, const size_t size, bool is_loose);

         known_script* _scan_changed_pex(
            known_script&  target,
            const uint8_t* data,
            const size_t   size,
            bool& out_info_changed,
            bool& out_hierarchy_changed
         );

         void _update_superclass_of(known_script&);

         known_script* _lookup_known_script(std::string_view scriptname) const {
            return const_cast<known_script*>(std::as_const(*this).lookup_known_script(scriptname));
         }

      public:
         // NOTE: Known-script pointers received from here should be considered invalid/undefined/dangling 
         //       once DovahKitCore::dataAbandonComplete fires. If you place them in smart pointers, you 
         //       must clear those on DovahKitCore::dataAbandonImminent.
         const known_script* lookup_known_script(std::string_view scriptname) const;
         const known_script* lookup_known_script(QString scriptname) const;

         void index_all_pex_files();

      protected:
         void _begin_watching_loose_pexs();
         void _stop_watching_loose_pexs();
         void _check_for_loose_pex_updates();
         void _on_all_loose_pexs_deleted();

      signals:
         void pexIndexingComplete();

         // Signals emitted after initial PEX indexing is complete:
         void knownScriptDiscovered(const known_script&);
         void knownScriptChanged(const known_script&);
         void knownScriptAboutToBeForgotten(const known_script& script); // you MUST sever any refcounting pointers to `script` upon receipt of this signal
         void knownScriptForgotten(std::string scriptname);

         void pexTeardownImminent(); // emitted before deleting all known scripts; you MUST sever any refcounting pointers to them upon receipt of this signal
         void pexTeardownComplete();
   };
}
