#pragma once
#include <cstdint>
#include <filesystem>
#include <unordered_map>
#include <QWidget>

namespace dovah {
   class bsa_archived_file;
   class compiled_papyrus_script;
}

class DovahKitPapyrusDictionary {
   public:
      static DovahKitPapyrusDictionary& get() {
         static DovahKitPapyrusDictionary instance;
         return instance;
      }
      using file_ptr_t = dovah::bsa_archived_file;
      using script_t   = dovah::compiled_papyrus_script;
      //
   protected:
      struct _std_hash_filesystem_path {
         size_t operator()(const std::filesystem::path& p) const { return std::filesystem::hash_value(p); }
      };
      struct known_file {
         script_t* data     = nullptr;
         uint32_t  refcount = 0;
      };

      std::unordered_map<std::filesystem::path, known_file, _std_hash_filesystem_path> known_files;

      void _register_destroy_handler(QWidget*, const std::filesystem::path&);

   public:
      //
      // Attempts to retrieve the compiled script data at the specified path. If successful, returns 
      // a pointer to the data and registers a handler to track the refcount, decreasing said refcount 
      // when the passed-in widget is destroyed. The path should be inside of the "Data" folder and 
      // relative to it.
      //
      script_t* get_script_for(QWidget*, const std::filesystem::path&);

      script_t* get_script_for(QWidget*, const QString& name);
};