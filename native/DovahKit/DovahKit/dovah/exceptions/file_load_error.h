#pragma once
#include <filesystem>
#include <stdexcept>

namespace dovah::exceptions {
   class file_load_error : public std::runtime_error {
      public:
         file_load_error(
            const std::filesystem::path& path
         ) :
            std::runtime_error("file load error"),
            file_path(path)
         {}

      public:
         std::filesystem::path file_path;
   };
}