#include "./file.h"
#include "lu/ascii/equals_i.h"

namespace dovah::tes {
   void file::open(const std::filesystem::path& path) {
      this->path = path;
      this->file.open(path);
      {
         this->app_flags.unset_each_of<file_app_specific_flag::file_extension_forces_light, file_app_specific_flag::file_extension_forces_master>();
         auto ext = path.extension().string();
         if (ext.size() == 4) {
            if (lu::ascii::equals_i(ext, ".esl")) {
               this->app_flags.set_each_of<file_app_specific_flag::file_extension_forces_light, file_app_specific_flag::file_extension_forces_master>();
            } else if (lu::ascii::equals_i(ext, ".esm")) {
               this->app_flags.set(file_app_specific_flag::file_extension_forces_master);
            }
         }
      }
   }
   void file::close() {
      this->file = {};
   }
   void file::reopen() {
      this->reopen(this->path);
   }
   void file::reopen(const std::filesystem::path& path) {
      this->file = {};
      this->file.open(path);
   }
}