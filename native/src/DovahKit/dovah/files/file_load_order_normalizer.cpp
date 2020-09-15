#include "file_load_order_normalizer.h"
#include "../../helpers/strings.h"
#include "file_header.h"

namespace dovah {
   bool file_load_order_normalizer::has_master(const std::string& name) const {
      auto& list = this->masters;
      for (auto it = list.begin(); it != list.end(); ++it)
         if (cobb::strieq(name, (*it)->name))
            return true;
      return false;
   }
   bool file_load_order_normalizer::has_plugin(const std::string& name) const {
      auto& list = this->plugins;
      for (auto it = list.begin(); it != list.end(); ++it)
         if (cobb::strieq(name, (*it)->name))
            return true;
      return false;
   }
   void file_load_order_normalizer::move_to_masters(const std::string& name) noexcept {
      auto& list = this->plugins;
      auto  it   = std::find_if(list.begin(), list.end(), [&name](file_header* file) { return cobb::strieq(name, file->name); });
      if (it == list.end())
         return;
      file_header* header = *it;
      list.erase(it);
      //
      auto& masterNames = header->masters;
      for (auto jt = masterNames.begin(); jt != masterNames.end(); ++jt) {
         this->move_to_masters(*jt);
      }
      this->masters.push_back(header);
   }
   bool file_load_order_normalizer::add(file_read_error& error, const std::string& name, bool isMasterOfMaster) {
      error.code = file_read_error::error_code::none;
      if (this->contains(name)) {
         //
         // File is already in the normalized load order, probably because it was selected to load 
         // after one of its masters and therefore added when we saw that master.
         //
         return true;
      }
      //
      auto header = new file_header;
      std::string path = this->base_path + name;
      if (!header->load(path.c_str())) {
         error.code       = file_read_error::error_code::malformed_file;
         error.file       = name;
         error.fileOffset = header->error.fileOffset;
         error.message    = "Failed initial read of the file header. ";
         error.message    += header->error.message;
         delete header;
         return false;
      }
      //
      if (this->seen.find(header->name) != seen.end()) {
         error.code    = file_read_error::error_code::cyclical_dependency_between_files;
         error.file    = name;
         error.message = "Failed initial read of the file header. This file is part of a cyclical dependency.";
         //
         auto& list = this->seen;
         if (list.size() > 1) {
            auto last = list.rbegin();
            error.dependency = *last;
         }
         //
         delete header;
         return false;
      }
      this->seen.insert(header->name);
      //
      bool must_be_master = isMasterOfMaster || header->is_master();
      for (auto it = header->masters.begin(); it != header->masters.end(); ++it) {
         if (!this->active_file.empty() && cobb::strieq(*it, this->active_file)) {
            auto& name = *it;
            error.code       = file_read_error::error_code::active_file_is_dependency;
            error.file       = name;
            error.dependency = this->active_file;
            error.message    = "The active file cannot be the master to another file in the load order.";
            this->seen.erase(header->name);
            delete header;
            return false;
         }
         if (this->has_master(*it))
            continue;
         if (this->has_plugin(*it)) {
            if (must_be_master)
               //
               // We allow ESPs and ESMs to be mixed together. This, of course, means that we need 
               // to handle the possibility of a list of queued files containing an ESP, followed 
               // by an ESM that has that ESP as a master.
               //
               this->move_to_masters(*it);
            continue;
         }
         //
         // If we got here, then we have an unexpected master. Recurse on it -- deal with any 
         // unexpected masters of the unexpected master.
         //
         if (!this->add(error, *it, must_be_master)) {
            this->seen.erase(header->name);
            delete header;
            return false;
         }
      }
      //
      // And now that we know all of (header)'s masters are in the load order, add (header) itself.
      //
      if (must_be_master)
         this->masters.push_back(header);
      else
         this->plugins.push_back(header);
      this->seen.erase(header->name);
      //
      if (this->size() > 254) {
         error.code    = file_read_error::error_code::too_many_files;
         error.file    = name;
         error.message = "The load order is too long.";
         this->seen.erase(header->name);
         delete header;
         return false;
      }
      return true;
   }
   bool file_load_order_normalizer::contains(const std::string& name) const noexcept {
      for (auto* file : this->masters)
         if (cobb::strieq(file->name, name))
            return true;
      for (auto* file : this->plugins)
         if (cobb::strieq(file->name, name))
            return true;
      return false;
   }
   void file_load_order_normalizer::delete_contents() {
      for (auto* header : this->masters)
         delete header;
      this->masters.clear();
      for (auto* header : this->plugins)
         delete header;
      this->plugins.clear();
   }
}