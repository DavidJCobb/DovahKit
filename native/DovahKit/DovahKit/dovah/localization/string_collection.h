#pragma once
#include <optional>
#include <span>
#include <string_view>
#include <vector>
#include "lu/passkey.h"
#include "../types/bare_localized_string_id.h"
#include "../types/localized_string_type.h"
#include "./language.h"
namespace dovah {
   namespace bsa {
      class archive_collection;
   }
   namespace localization {
      class active_string_file_bundle;
      class string_file_bundle;
   }
   namespace tes {
      class file;
   }
   class active_load_order;
}

namespace dovah::localization {
   class string_collection {
      public:
         using const_string_view = std::basic_string_view<const char>;

         ~string_collection();

      protected:
         std::vector<string_file_bundle*> _non_active_files;
         active_string_file_bundle* _active_file = nullptr;

      public:
         const active_string_file_bundle* bundle_for_active_file() const;
         const string_file_bundle* bundle_for_non_active_file(const tes::file&) const;
         std::optional<const_string_view> lookup(const tes::file&, localized_string_type, language, bare_localized_string_id) const noexcept;

      public: // passkeyed
         void _load(lu::passkey<active_load_order>, const dovah::bsa::archive_collection&, std::span<const tes::file* const>, const tes::file* active_file);
   };
}