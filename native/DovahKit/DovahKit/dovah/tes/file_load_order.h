#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "../game.h"
#include "./file_header_flags.h"
namespace dovah::tes {
   class file_queue;
}

namespace dovah::tes {
   //
   // Given a `file_queue`, this class generates an ordered list of files, accounting 
   // for master-flagged files, dependencies, and so on.
   // 
   // This only validates whether it is possible to load the given list of files. It 
   // does not validate whether it'd be possible to save an active file given the list 
   // of files. For example, you can load thousands of files if most of them are light 
   // data files, but you wouldn't be able to save an active file because (as of this 
   // writing) we add every loaded file as a dependency, and a file can only have up 
   // to 254 dependencies. This class does not check that constraint.
   //
   class file_load_order {
      public:
         using file_name_type = std::string;

         struct file_info {
            std::vector<file_name_type> dependencies;
            file_name_type              filename;
            float                       file_version           = 0.0F;
            file_header_flags           flags;
            uint32_t                    record_and_group_count = 0;

            constexpr bool is_light() const noexcept {
               return this->flags.has(file_header_flag::light);
            }
            constexpr bool is_master() const noexcept {
               return this->flags.has(file_header_flag::master);
            }
         };

      protected:
         std::set<file_name_type> seen; // used to detect cyclical dependencies between files
      public:
         struct {
            file_name_type        active_file_name;
            std::filesystem::path data_directory;
            game                  target_game = game::skyrim_special;
         } config;
         std::vector<std::unique_ptr<file_info>> masters;
         std::vector<std::unique_ptr<file_info>> plugins;

      public:
         file_load_order() {}
         file_load_order(game, const std::filesystem::path& data_directory, const file_queue&);
         file_load_order(game, std::filesystem::path&& data_directory, const file_queue&);

         bool has_master(const file_name_type& filename) const noexcept;
         bool has_plugin(const file_name_type& filename) const noexcept;
         void move_to_masters(const file_name_type& name);
         
         constexpr uint16_t size() const noexcept {
            return (uint16_t)(this->masters.size() + this->plugins.size());
         }
         
         void add(const file_name_type& name, bool is_master_of_master = false);
         void add(const file_queue&);

         bool contains(const file_name_type& name) const noexcept;
   };
}