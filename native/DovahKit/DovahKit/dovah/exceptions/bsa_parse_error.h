#pragma once
#include <filesystem>
#include <stdexcept>

namespace dovah::exceptions {
   class bsa_parse_error : public std::runtime_error {
      public:
         bsa_parse_error(
            const std::filesystem::path& archive_path
         ) :
            std::runtime_error("BSA load error"),
            archive_path(archive_path)
         {}

      public:
         std::filesystem::path archive_path;
   };
}