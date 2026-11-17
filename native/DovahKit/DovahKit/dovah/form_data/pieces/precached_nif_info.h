#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
#include "lu/bytestreams/customization/serialized_bytecount.h"
namespace dovah::tes {
   class subrecord_view;
}

namespace dovah::form_data::pieces {
   class precached_nif_info {
      public:
         struct file_id {
            public:
               constexpr file_id() {
                  this->set_extension("dds");
               }

               // NOTE: There should not be redundant path separators nor a data-directory prefix in the folder path, nor a period in the extension.
               template<size_t ExtensionLength> requires (ExtensionLength <= 4)
               constexpr file_id(std::string_view folder, std::string_view filename, const char(&extension)[ExtensionLength]);

               // NOTE: There should not be redundant path separators or a data-directory prefix in the given path.
               constexpr file_id(std::string_view full_path);

            public:
               uint32_t file_hash   = 0;
               uint32_t extension   = 0;
               uint32_t folder_hash = 0;

            public:
               template<size_t ExtensionLength> requires (ExtensionLength <= 4)
               constexpr void set_extension(const char(&)[ExtensionLength]);

               constexpr bool operator==(const file_id&) const noexcept = default;

               bool stream(tes::subrecord_view& subrecord);
               void unchecked_stream(tes::subrecord_view& subrecord);
         };

      public:
         std::vector<file_id>  texture_hashes;
         std::vector<uint32_t> addon_node_ids;
         std::vector<file_id>  material_hashes;

      public:
         constexpr bool empty() const noexcept {
            if (!this->texture_hashes.empty())
               return false;
            if (!this->addon_node_ids.empty())
               return false;
            if (!this->material_hashes.empty())
               return false;
            return true;
         }
   };
}

namespace lu::bytestreams {
   template<typename Stream>
   struct serialized_bytecount<Stream, dovah::form_data::pieces::precached_nif_info::file_id> {
      private:
         using value_type = dovah::form_data::pieces::precached_nif_info::file_id;
      public:
         static constexpr const size_t value = 0xC;
   };
}

#include "./precached_nif_info.inl"