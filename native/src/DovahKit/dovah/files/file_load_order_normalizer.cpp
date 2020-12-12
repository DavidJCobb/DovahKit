#include "file_load_order_normalizer.h"
#include "../../helpers/strings.h"
#include "tes_file_reading/file_header.h"
#include "../detailed_notice.h"
#include "../notice_code_list.h"

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
      auto  it   = std::find_if(list.begin(), list.end(), [&name](file_header_reader* file) { return cobb::strieq(name, file->name); });
      if (it == list.end())
         return;
      file_header_reader* header = *it;
      list.erase(it);
      //
      auto& masterNames = header->masters;
      for (auto jt = masterNames.begin(); jt != masterNames.end(); ++jt) {
         this->move_to_masters(*jt);
      }
      this->masters.push_back(header);
   }
   bool file_load_order_normalizer::add(detailed_notice& error, const std::string& name, bool isMasterOfMaster) {
      error.code = notice_code::none;
      if (this->contains(name)) {
         //
         // File is already in the normalized load order, probably because it was selected to load 
         // after one of its masters and therefore added when we saw that master.
         //
         return true;
      }
      //
      auto header = new file_header_reader;
      std::string path = this->base_path + name;
      if (!header->load(path.c_str(), &error)) {
         error.code = notice_code::malformed_file;
         delete header;
         return false;
      }
      //
      if (this->seen.find(header->name) != seen.end()) {
         error.code = notice_code::cyclical_dependency_between_files;
         error.set_cause_file(name);
         //
         auto& list = this->seen;
         if (list.size() > 1) {
            auto last = list.rbegin();
            error.add_relevant_file(*last);
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
            error.code = notice_code::active_file_is_dependency;
            error.set_cause_file(name);
            error.add_relevant_file(this->active_file);
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
      // We now know for sure that all of (header)'s masters are in the load order. Let's wrap this 
      // up.
      //
      this->seen.erase(header->name);
      //
      if (!this->active_file.empty() && this->size() > 255) {
         error.code = notice_code::load_order_would_have_too_many_files;
         error.set_cause_file(name);
         error.extra_integers[0] = this->size();
         //error.message = "If an active file is selected, then the load order cannot contain more than 255 files (even if some of them are ESLs). When the active file is saved, all loaded files will be encoded as its masters, and the file format doesn't actually support ESL functionality when encoding form IDs, so loading this many files would cause form IDs in the active file to overflow into the 0xFF slot after saving.";
      }
      if (!game_supports_light_plugins(this->target_game)) {
         if (this->size() > 254) {
            error.code = notice_code::load_order_would_have_too_many_files;
            error.set_cause_file(name);
            error.extra_integers[0] = this->size();
         }
      } else {
         int16_t light_count = -1;
         int16_t heavy_count = -1;
         for (auto& header : this->masters) {
            if (header->is_light())
               ++light_count;
            else
               ++heavy_count;
         }
         for (auto& header : this->plugins) {
            if (header->is_light())
               ++light_count;
            else
               ++heavy_count;
         }
         if (light_count > 4096) {
            error.code = notice_code::load_order_would_have_too_many_files;
            error.set_cause_file(name);
         }
         if (heavy_count > 255) {
            error.code = notice_code::load_order_would_have_too_many_files;
            error.set_cause_file(name);
         }
         if (error.code == notice_code::load_order_would_have_too_many_files) {
            error.extra_integers[0] = this->size();
            error.extra_integers[1] = heavy_count;
            error.extra_integers[2] = light_count;
         }
      }
      if (error.is_defined()) {
         delete header;
         return false;
      }
      //
      if (must_be_master)
         this->masters.push_back(header);
      else
         this->plugins.push_back(header);
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