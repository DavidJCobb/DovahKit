#include "localized_string_store.h"
#include <limits>
#include "../files/bsa/bsa_load_order.h"
#include "../../helpers/unordered_map.h"
#include "../../helpers/unreachable.h"

namespace {
   static const std::basic_string_view<const char> _missing_string = "<MISSING STRING>";
}

namespace dovah {
   #pragma region localized_string_store::language_content
      void localized_string_store::language_content::load(localized_string_store& owner) {
         if (this->loaded)
            return;
         this->_load_single_file(owner, localized_string_type::common);
         this->_load_single_file(owner, localized_string_type::description);
         this->_load_single_file(owner, localized_string_type::info);
         this->loaded = true;
      }
                
      void localized_string_store::language_content::_load_single_file(localized_string_store& owner, localized_string_type type) {
         auto filename = owner.tes_filename;
         auto bare     = filename.stem().string();
         bare += '_';
         bare += this->name;
         switch (type) {
            case localized_string_type::common:      bare += ".strings";   break;
            case localized_string_type::description: bare += ".dlstrings"; break;
            case localized_string_type::info:        bare += ".ilstrings"; break;
            default:
               cobb::unreachable();
         }
         filename.replace_filename(bare);
         filename = std::filesystem::path(L"strings") / filename;
         //
         auto* file = owner.file_source.lookup_file(filename.string(), true);
         if (!file)
            return;
         //
         uint32_t count;
         file->read(0, count); // 00: number of strings
         // 04: size of all string data
         
         constexpr size_t size_of_header = 8;
         constexpr size_t size_of_entry  = 8;
         auto& dst_file_info = [this, type]() -> string_file& {
            switch (type) {
               case localized_string_type::common: return this->files_by_type.general;
               case localized_string_type::description: return this->files_by_type.description;
               case localized_string_type::info: return this->files_by_type.info;
            }
            std::unreachable();
         }();
         dst_file_info.file.reset(file);

         auto& entries = dst_file_info.strings_by_id;
         entries[0] = { 0, 0 };
         size_t data_offset = size_of_header + (count * size_of_entry);
         for (uint32_t i = 0; i < count; ++i) {
            id_t     id;
            uint32_t offset;
            file->read(size_of_header + (size_of_entry * i) + 0,          id);
            file->read(size_of_header + (size_of_entry * i) + sizeof(id), offset);
            //
            auto& string = entries[id];
            offset += data_offset;
            if (type != localized_string_type::common) {
               uint32_t size;
               file->read(offset, size);
               offset += sizeof(size);
               if (offset + size <= file->size()) {
                  entries[id] = { offset, size };
               }
            } else {
               uint32_t size = (uint32_t)strnlen_s(
                  (const char*)file->data() + offset,
                  std::min(
                     file->size() - offset,
                     (size_t)std::numeric_limits<uint32_t>::max()
                  )
               );
               entries[id] = { offset, size };
            }
         }
      }
   #pragma endregion

   localized_string_store::localized_string_store(bsa_load_order& blo, const std::filesystem::path& tfn) : file_source(blo), tes_filename(tfn) {
   }
   localized_string_store::~localized_string_store() {
   }

   void localized_string_store::set_default_language(const language_name_t& n) {
      this->default_language      = n;
      this->default_language_enum = language_name_to_localization_enum(n.c_str());
   }

   void localized_string_store::open_language_files(const language_name_t& language_name) {
      if (this->tes_filename.empty())
         return;
      //
      auto normalized_language = language_name;
      for (auto& c : normalized_language)
         c = tolower(c);
      //
      auto& language = this->content_by_language[normalized_language];
      if (!language.loaded) {
         language.name = normalized_language;
         language.load(*this);
      }
   }
   localized_string_store::const_string_view localized_string_store::lookup(localized_string_type type, const language_name_t& language_name, id_t id) {
      auto normalized_language = language_name;
      for (auto& c : normalized_language)
         c = tolower(c);
      if (!cobb::unordered_map_contains(this->content_by_language, normalized_language)) {
         //
         // TODO: should we load the language on demand?
         //
         return _missing_string; // for now, let's not
      }

      auto& language  = this->content_by_language[normalized_language];
      auto& file_info = [&language, type]() -> const string_file& {
         switch (type) {
            case localized_string_type::common:      return language.files_by_type.general;
            case localized_string_type::description: return language.files_by_type.description;
            case localized_string_type::info:        return language.files_by_type.info;
         }
         std::unreachable();
      }();

      auto it = file_info.strings_by_id.find(id);
      if (it == file_info.strings_by_id.end())
         return _missing_string;
      
      return const_string_view(
         (const char*)file_info.file->data() + it->second.offset,
         it->second.size
      );
   }
}