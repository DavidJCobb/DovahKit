#pragma once
#include <filesystem>
#include <stdexcept>

namespace dovah::exceptions {
   // Failure to extract a packed file from a BSA.
   class bsa_extract_error : public std::runtime_error {
      public:
         bsa_extract_error(
            const std::filesystem::path& archive_path
         ) :
            std::runtime_error("BSA extract error"),
            archive_path(archive_path)
         {}

      public:
         std::filesystem::path archive_path;
   };
}