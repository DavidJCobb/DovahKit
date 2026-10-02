#include "./file_load_order.h"
#include "lu/ascii/equals_i.h"
#include "dovah/facts/game/game_supports_light_plugins.h"
#include "./file_queue.h"
#include "./skimming/file_header_skimmer.h"

#include "dovah/exceptions/load_order_errors/cyclical_dependency_between_files.h"
#include "dovah/exceptions/load_order_errors/desired_active_file_is_a_dependency.h"
#include "dovah/exceptions/load_order_errors/load_order_would_have_too_many_files.h"

namespace dovah::tes {
   file_load_order::file_load_order(game g, const std::filesystem::path& data_directory, const file_queue& src) :
      config({
         .active_file_name = src.get_active_file_name(),
         .data_directory   = data_directory,
         .target_game = g,
      })
   {
      this->add(src);
   }
   file_load_order::file_load_order(game g, std::filesystem::path&& data_directory, const file_queue& src) :
      config({
         .active_file_name = src.get_active_file_name(),
         .data_directory   = std::move(data_directory),
         .target_game = g,
      })
   {
      this->add(src);
   }

   bool file_load_order::has_master(const std::string& name) const noexcept {
      for (auto& info_ptr : this->masters)
         if (lu::ascii::equals_i(info_ptr->filename, name))
            return true;
      return false;
   }
   bool file_load_order::has_plugin(const std::string& name) const noexcept {
      for (auto& info_ptr : this->plugins)
         if (lu::ascii::equals_i(info_ptr->filename, name))
            return true;
      return false;
   }
   void file_load_order::move_to_masters(const std::string& name) {
      auto& list = this->plugins;
      auto  it   = std::find_if(
         list.begin(),
         list.end(),
         [&name](auto& info_ptr) {
            return lu::ascii::equals_i(name, info_ptr->filename);
         }
      );
      if (it == list.end())
         return;

      auto file_ptr = std::move(*it);
      list.erase(it);
      for (auto& dependency_filename : file_ptr->dependencies)
         this->move_to_masters(dependency_filename);
      this->masters.push_back(std::move(file_ptr));
   }
   void file_load_order::add(const std::string& name, bool is_master_of_master) {
      if (this->contains(name)) {
         //
         // File is already in the normalized load order, probably because it was selected to load 
         // after one of its masters and therefore added when we saw that master.
         //
         return;
      }
      if (this->seen.find(name) != seen.end()) {
         throw exceptions::load_order_errors::cyclical_dependency_between_files(
            this->config.data_directory,
            this->config.target_game,
            this->seen,
            name
         );
      }
      this->seen.insert(name);

      file_header_skimmer skimmer;
      skimmer.skim(this->config.data_directory / name);

      bool must_be_master = is_master_of_master || skimmer.results.flags.has(file_header_flag::master);
      for (auto& dependency : skimmer.results.dependencies) {
         if (!this->config.active_file_name.empty() && lu::ascii::equals_i(dependency, this->config.active_file_name)) {
            throw exceptions::load_order_errors::desired_active_file_is_a_dependency(
               this->config.data_directory,
               this->config.target_game,
               this->config.active_file_name,
               name
            );
         }
         if (this->has_master(dependency))
            continue;
         if (this->has_plugin(dependency)) {
            if (must_be_master)
               //
               // We allow ESPs and ESMs to be mixed together. This, of course, means that we need 
               // to handle the possibility of a list of queued files containing an ESP, followed 
               // by an ESM that has that ESP as a master.
               //
               this->move_to_masters(dependency);
            continue;
         }
         //
         // If we got here, then we have an unexpected master. Recurse on it -- deal with any 
         // unexpected masters of the unexpected master.
         //
         this->add(dependency, must_be_master);
      }
      //
      // We now know for sure that all of (header)'s masters are in the load order. Let's wrap this 
      // up.
      //
      this->seen.erase(name);

      if (!facts::game_supports_light_plugins(this->config.target_game)) {
         if (this->size() > 254) {
            auto ex = exceptions::load_order_errors::load_order_would_have_too_many_files(this->config.data_directory, this->config.target_game);
            ex.file_counts.heavy  = this->size();
            ex.file_counts.total  = this->size();
            ex.overflowed_at_file = name;
            throw ex;
         }
      } else {
         int16_t light_count = 0;
         int16_t heavy_count = 0;
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
         if (light_count > 4095 || heavy_count > 255) {
            auto ex = exceptions::load_order_errors::load_order_would_have_too_many_files(this->config.data_directory, this->config.target_game);
            ex.file_counts.heavy  = heavy_count;
            ex.file_counts.light  = light_count;
            ex.file_counts.total  = this->size();
            ex.overflowed_at_file = name;
            throw ex;
         }
      }

      auto info_ptr = std::make_unique<file_info>();
      info_ptr->dependencies           = std::move(skimmer.results.dependencies);
      info_ptr->filename               = name;
      info_ptr->file_version           = skimmer.results.file_version;
      info_ptr->flags                  = skimmer.results.flags;
      info_ptr->record_and_group_count = skimmer.results.record_and_group_count;
      ((must_be_master) ? this->masters : this->plugins).push_back(std::move(info_ptr));
   }
   void file_load_order::add(const file_queue& queue) {
      for (const auto& file_path : queue.get_file_list()) {
         this->add(file_path, false);
      }
   }

   bool file_load_order::contains(const std::string& name) const noexcept {
      if (this->has_master(name))
         return true;
      if (this->has_plugin(name))
         return true;
      return false;
   }
}