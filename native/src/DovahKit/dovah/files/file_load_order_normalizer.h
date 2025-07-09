#pragma once
#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include "../data/game.h"

namespace dovah {
   namespace tes_file_reading {
      class file_header_reader;
   }

   class file_load_order_normalizer {
      //
      // This class generates a normalized load order -- that is, a load order 
      // that includes any unexpected masters, and that reorders files as app-
      // ropriate. If, for example, you were to add files in this order:
      //
      //    Dawnguard.esm
      //    Skyrim.esm
      //
      // This class would end up containing:
      //
      //    Skyrim.esm
      //    Update.esm
      //    Dawnguard.esm
      //
      // It handles unexpected masters, out-of-order dependencies, and files 
      // that need to be reordered due to having the master flag.
      //
      public:
         using file_header_reader = tes_file_reading::file_header_reader;
         //
         std::set<std::string> seen; // used to detect cyclical dependencies between files
         std::vector<file_header_reader*> masters;
         std::vector<file_header_reader*> plugins;
         std::string base_path;
         std::string active_file;
         game        target_game = game::skyrim_special;
         //
         bool has_master(const std::string& filename) const;
         bool has_plugin(const std::string& filename) const;
         void move_to_masters(const std::string& name) noexcept;
         //
         inline uint16_t size() const noexcept {
            return this->masters.size() + this->plugins.size();
         }
         
         void add(const std::string& name, bool is_master_of_master = false);

         bool contains(const std::string& name) const noexcept;
         
         void delete_contents();
   };
}