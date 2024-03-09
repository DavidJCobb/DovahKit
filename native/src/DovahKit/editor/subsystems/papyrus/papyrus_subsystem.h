#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include "helpers/passkey.h"
#include "helpers/singleton_ex.h"
#include "./known_script_ptr.h"

namespace dovah {
   namespace loaded_forms {
      class Alias;
   }
   class form_stub;
}

namespace dovahkit::subsystems::papyrus {
   class known_script;

   class core;
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      private:
         struct _self_passkey {};

      protected:
         std::mutex _vmad_scanning_mutex;
         std::unordered_map<std::string, known_script*> _known_scripts_by_name; // keys are forced to ASCII-lowercase
         struct {
            bool    folder_exists = false;
            QString folder_path;
            QFileSystemWatcher watcher;
         } _loose_pex;
         bool _initial_discovery_complete = false;
         bool _teardown_in_progress       = false;

         void _teardown();

         static std::string _normalize_scriptname(std::string_view);
         static std::string _normalize_scriptname(QString);

         // Scan a PEX when we don't know if we've already seen the script, or when we know for certain that 
         // it's a newly-created PEX file for a script we've never seen before.
         known_script* _scan_pex(const std::string& filename_sans_ext, const uint8_t* data, const size_t size, bool is_loose, bool is_loose_file_creation);

         // Scan a PEX when we know for certain that we've seen the script before.
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

         template<typename Functor> requires (std::is_invocable_v<Functor, const known_script&>)
         void for_each_known_script(Functor&& functor) const {
            for (auto& pair : this->_known_scripts_by_name)
               functor(*(const known_script*)pair.second);
         }

         constexpr bool is_initial_script_discovery_complete() const noexcept {
            return this->_initial_discovery_complete;
         }

         // If a form has a script attached, then we know about that script by virtue of that attachment. 
         // As such, these member functions look up the script, and create it if it isn't known.
         // 
         // The `form_info_cache` subsystem is responsible for scanning all forms on load, skimming their 
         // VMAD subrecords, and building a map of form stubs to lists of known scripts. It will maintain 
         // that map as the user makes edits to forms. That subsystem should call these functions when it 
         // sees that a form has a script attached: that subsystem should affirmatively choose to "know" 
         // the script whose name it sees.
         //
         known_script_ptr know_script(std::string_view scriptname);
         known_script_ptr know_script_via_vmad_scan(std::string_view scriptname);

         void _on_script_unreferenced(cobb::passkey<known_script, core>, known_script&);

      protected:
         void _index_all_pex_files();

      public:
         bool form_has_script_attached(const dovah::form_stub&, std::string_view scriptname) const;
         bool quest_has_script_attached_to_any_alias(const dovah::form_stub& quest, std::string_view scriptname) const;

         // Prefer this over checking the alias's script list; this handles subclass/superclass relationships 
         // amongst Papyrus scripts.
         bool quest_alias_has_script_attached(const dovah::loaded_forms::Alias&, std::string_view scriptname) const;

      protected:
         void _begin_watching_loose_pexs();
         void _stop_watching_loose_pexs();
         void _check_for_loose_pex_updates();
         void _on_all_loose_pexs_deleted();

      signals:
         void pexIndexingComplete();
         void initialKnownScriptDiscoveryComplete(); // PEXs indexed and VMADs scanned

         // Signals emitted after initial PEX indexing is complete:
         void knownScriptDiscovered(const known_script&);
         void knownScriptChanged(const known_script&);
         void knownScriptAboutToBeForgotten(const known_script& script); // you MUST sever any refcounting pointers to `script` upon receipt of this signal
         void knownScriptForgotten(std::string scriptname);

         void pexTeardownImminent(); // emitted before deleting all known scripts; you MUST sever any refcounting pointers to them upon receipt of this signal
         void pexTeardownComplete();

         void _scriptUnreferencedDeferToMainThread(_self_passkey, known_script&); // see: _on_script_unreferenced
   };
}
