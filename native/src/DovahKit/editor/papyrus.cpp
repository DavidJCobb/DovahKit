#include "papyrus.h"
#include "../helpers/unordered_map.h"
#include "../dovah/files/bsa/bsa_archived_file.h"
#include "../dovah/files/papyrus/compiled_script.h"
#include "core.h"

void DovahKitPapyrusDictionary::_register_destroy_handler(QWidget* owner, const std::filesystem::path& path) {
   QObject::connect(owner, &QObject::destroyed, [this, path]() {
      auto& entry = this->known_files[path];
      assert(entry.refcount);
      if (--entry.refcount)
         return;
      delete entry.data;
      this->known_files.erase(path);
   });
}

DovahKitPapyrusDictionary::script_t* DovahKitPapyrusDictionary::get_script_for(QWidget* owner, const std::filesystem::path& path) {
   auto& entry = this->known_files[path];
   if (!entry.data) {
      auto* script = DovahKitCore::get().lookup_game_asset(path);
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
   auto& editor = DovahKitCore::get();
   std::filesystem::path path = "scripts";
   path.append(name.toStdWString());
   path.replace_extension("pex");
   return this->get_script_for(widget, path);
}