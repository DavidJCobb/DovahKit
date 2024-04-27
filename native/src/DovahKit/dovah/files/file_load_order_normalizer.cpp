#include "./file_load_order_normalizer.h"
#include <memory>
#include "helpers/string/strieq_ascii.h"
#include "./tes_file_reading/file_header.h"

#include "../exceptions/invalid_load_order/cyclical_dependency_between_files.h"
#include "../exceptions/invalid_load_order/desired_active_file_is_a_dependency.h"
#include "../exceptions/invalid_load_order/load_order_would_have_too_many_files.h"
#include "../exceptions/invalid_load_order.h"

namespace dovah {
   bool file_load_order_normalizer::has_master(const std::string& name) const {
      auto& list = this->masters;
      for (auto it = list.begin(); it != list.end(); ++it)
         if (cobb::strieq_ascii(name, (*it)->name))
            return true;
      return false;
   }
   bool file_load_order_normalizer::has_plugin(const std::string& name) const {
      auto& list = this->plugins;
      for (auto it = list.begin(); it != list.end(); ++it)
         if (cobb::strieq_ascii(name, (*it)->name))
            return true;
      return false;
   }
   void file_load_order_normalizer::move_to_masters(const std::string& name) noexcept {
      auto& list = this->plugins;
      auto  it   = std::find_if(list.begin(), list.end(), [&name](file_header_reader* file) { return cobb::strieq_ascii(name, file->name); });
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
   void file_load_order_normalizer::add(const std::string& name, bool isMasterOfMaster) {
      if (this->contains(name)) {
         //
         // File is already in the normalized load order, probably because it was selected to load 
         // after one of its masters and therefore added when we saw that master.
         //
         return;
      }
      //
      auto header = std::make_unique<file_header_reader>();
      std::string path = this->base_path + name;
      header->load(path.c_str());
      //
      if (this->seen.find(header->name) != seen.end()) {
         auto ex = exceptions::invalid_load_order_exceptions::cyclical_dependency_between_files();
         for (auto& item : this->seen) {
            ex.seen.push_back(item);
         }
         ex.seen.push_back(header->name); // so one can tell where the cycle was
         throw ex;
      }
      this->seen.insert(header->name);
      //
      bool must_be_master = isMasterOfMaster || header->is_master();
      for (auto it = header->masters.begin(); it != header->masters.end(); ++it) {
         if (!this->active_file.empty() && cobb::strieq_ascii(*it, this->active_file)) {
            auto& name = *it;

            auto ex = exceptions::invalid_load_order_exceptions::desired_active_file_is_a_dependency();
            ex.active_file    = this->active_file;
            ex.dependent_file = name;
            throw ex;
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
         this->add(*it, must_be_master);
      }
      //
      // We now know for sure that all of (header)'s masters are in the load order. Let's wrap this 
      // up.
      //
      this->seen.erase(header->name);
      //
      if (!this->active_file.empty() && this->size() > 255) {
         auto ex = exceptions::invalid_load_order_exceptions::load_order_would_have_too_many_files();
         ex.file_counts.active_file_dependencies = this->size();
         ex.file_counts.total  = this->size();
         ex.overflowed_at_file = name;
         throw ex;
         //error.message = "If an active file is selected, then the load order cannot contain more than 255 files (even if some of them are ESLs). When the active file is saved, all loaded files will be encoded as its masters, and the file format doesn't actually support ESL functionality when encoding form IDs, so loading this many files would cause form IDs in the active file to overflow into the 0xFF slot after saving.";
      }
      if (!game_supports_light_plugins(this->target_game)) {
         if (this->size() > 254) {
            auto ex = exceptions::invalid_load_order_exceptions::load_order_would_have_too_many_files();
            ex.file_counts.heavy  = this->size();
            ex.file_counts.total  = this->size();
            ex.overflowed_at_file = name;
            throw ex;
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
         if (light_count > 4096 || heavy_count > 255) {
            auto ex = exceptions::invalid_load_order_exceptions::load_order_would_have_too_many_files();
            ex.file_counts.heavy  = heavy_count;
            ex.file_counts.light  = light_count;
            ex.file_counts.total  = this->size();
            ex.overflowed_at_file = name;
            throw ex;
         }
      }
      
      if (must_be_master)
         this->masters.push_back(header.release());
      else
         this->plugins.push_back(header.release());
   }
   bool file_load_order_normalizer::contains(const std::string& name) const noexcept {
      for (auto* file : this->masters)
         if (cobb::strieq_ascii(file->name, name))
            return true;
      for (auto* file : this->plugins)
         if (cobb::strieq_ascii(file->name, name))
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