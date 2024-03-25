#pragma once
#include <bit>
#include <cstdint>
#include <string_view>
#include <vector>

namespace dovah::loaded_forms {
   struct precached_nif_info {
      struct file_id {
         uint32_t file_hash   = 0;
         uint32_t extension;
         uint32_t folder_hash = 0;

         constexpr file_id() {
            this->set_extension("dds");
         }

         // NOTE: There should not be redundant path separators nor a data-directory prefix in the folder path, nor a period in the extension.
         template<size_t ExtensionLength> requires (ExtensionLength <= 4)
         constexpr file_id(std::string_view folder, std::string_view filename, const char(&extension)[ExtensionLength]);

         // NOTE: There should not be redundant path separators or a data-directory prefix in the given path.
         constexpr file_id(std::string_view full_path);

         template<size_t ExtensionLength> requires (ExtensionLength <= 4)
         constexpr void set_extension(const char(&)[ExtensionLength]);

         constexpr bool operator==(const file_id&) const noexcept = default;
      };

      std::vector<file_id>  texture_hashes;
      std::vector<uint32_t> addon_node_ids;
      std::vector<file_id>  material_hashes;
   };
}

#include "./precached_nif_info.inl"