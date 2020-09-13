#pragma once
#include <set>
#include <string>
#include <vector>
#include "file_read_error.h"

namespace dovah {
   class file_header;

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
         std::set<std::string>     seen; // used to detect cyclical dependencies between files
         std::vector<file_header*> masters;
         std::vector<file_header*> plugins;
         std::string base_path;
         std::string active_file;
         //
         bool has_master(const std::string& filename) const;
         bool has_plugin(const std::string& filename) const;
         void move_to_masters(const std::string& name) noexcept;
         //
         inline uint16_t size() const noexcept {
            return this->masters.size() + this->plugins.size();
         }
         //
         bool add(file_read_error& out, const std::string& name, bool is_master_of_master = false);
         //
         void delete_contents();
   };
}