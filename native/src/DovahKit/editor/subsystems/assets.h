#pragma once
#include <filesystem>
#include <string>
#include "helpers/singleton_ex.h"
//
#include "dovah/files/papyrus/compiled_script.h"

// Forward-declarations:
namespace dovah {
   class bsa_archived_file;
}

namespace dovahkit::subsystems {
   class assets;

   //
   // Subsystem for accessing game assets.
   //
   class assets : public cobb::singleton_ex<assets> {
      protected:
         assets();
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         // The specified path should be relative to, and should not include, the Data directory. The 
         // caller should delete any returned object.
         dovah::bsa_archived_file* lookup_loose_game_asset(const std::filesystem::path&);

         // The specified path should be relative to, and should not include, the Data directory. The 
         // caller should delete any returned object.
         dovah::bsa_archived_file* lookup_game_asset(const std::filesystem::path&, bool allow_loose_files = true);

         // This can throw exceptions; see definition for dovah::compiled_papyrus_script.
         dovah::compiled_papyrus_script parse_compiled_script(const std::string& scriptname);
   };
}