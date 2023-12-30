#include "papyrus_dictionary.h"
#include "../helpers/unordered_map.h"
#include "../dovah/files/bsa/bsa_archived_file.h"
#include "../dovah/files/papyrus/compiled_script.h"
#include "./subsystems/assets.h"

/*static*/ std::filesystem::path DovahKitPapyrusDictionary::_path_from_scriptname(const QString& name) noexcept {
   std::filesystem::path path = "scripts";
   path.append(name.toStdWString());
   path.replace_extension("pex");
   return path;
}
void DovahKitPapyrusDictionary::_register_destroy_handler(QWidget* owner, const std::filesystem::path& path) {
   QObject::connect(
      owner,
      &QObject::destroyed,
      this,
      [this, path]() {
         auto& entry = this->known_files[path];
         assert(entry.refcount);
         if (--entry.refcount)
            return;
         delete entry.data;
         this->known_files.erase(path);
      },
      Qt::UniqueConnection
   );
}

DovahKitPapyrusDictionary::script_t* DovahKitPapyrusDictionary::get_script_for(QWidget* owner, const std::filesystem::path& path) {
   auto& entry = this->known_files[path];
   if (!entry.data) {
      auto* script = dovahkit::subsystems::assets::get().lookup_game_asset(path);
      if (script) {
         entry.data = new script_t;
         entry.data->read_file(script->data(), script->size());
         delete script;
      }
      if (!entry.data) {
         this->known_files.erase(path);
         return nullptr;
      }
   }
   ++entry.refcount;
   this->_register_destroy_handler(owner, path);
   return entry.data;
}
DovahKitPapyrusDictionary::script_t* DovahKitPapyrusDictionary::get_script_for(QWidget* widget, const QString& name) {
   return this->get_script_for(widget, _path_from_scriptname(name));
}

void DovahKitPapyrusDictionary::relinquish_script_from(QWidget* widget, const std::filesystem::path& path) {
   auto it = this->known_files.find(path);
   if (it == this->known_files.end())
      return;
   auto& pair  = *it;
   auto& entry = pair.second;
   if (!QObject::disconnect(widget, &QObject::destroyed, this, nullptr)) // (widget) never owned this script in the first place
      return;
   if (--entry.refcount)
      return;
   if (entry.data)
      delete entry.data;
   this->known_files.erase(it);
}
void DovahKitPapyrusDictionary::relinquish_script_from(QWidget* widget, const QString& name) {
   return this->relinquish_script_from(widget, _path_from_scriptname(name));
}