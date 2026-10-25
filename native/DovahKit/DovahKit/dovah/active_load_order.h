#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>
#include "lu/passkey.h"
#include "./form_stub_collection.h"
#include "./game_setting_collection.h"
#include "./game.h"
namespace dovah {
   namespace bsa {
      class archive_collection;
   }
   namespace callback_sets {
      struct data_load;
   }
   namespace localization {
      class string_file_bundle;
   }
   namespace tes {
      class file;
      class file_load_order;
   }
   struct file_prefix;
   class  form_stub;
}

namespace dovah {
   class active_load_order {
      public:
         active_load_order();
         ~active_load_order();

         active_load_order(const active_load_order&) = delete;
         active_load_order& operator=(const active_load_order&) = delete;

      protected:
         std::filesystem::path _data_directory;
         struct {
            std::vector<tes::file*> list; // owned
            tes::file*              active = nullptr; // unowned; also in `list`
            struct {
               tes::file* hardcoded  = nullptr;
               tes::file* none_stubs = nullptr;
            } dummies;
            std::vector<localization::string_file_bundle*> string_file_bundles;
         } _files;
         bsa::archive_collection* _bsa_collection = nullptr;
         form_stub_collection     _forms{ *this };
         game_setting_collection  _game_settings;
         game _game = game::skyrim_classic;

      public:
         bsa::archive_collection* get_or_create_bsa_collection() noexcept;
         constexpr const bsa::archive_collection* get_bsa_collection() const noexcept { return this->_bsa_collection; };
         constexpr bsa::archive_collection* get_bsa_collection() noexcept { return this->_bsa_collection; };

         constexpr const form_stub_collection& get_form_stub_collection() const noexcept { return this->_forms; };
         constexpr form_stub_collection& get_form_stub_collection() noexcept { return this->_forms; };

         constexpr const game_setting_collection& get_game_setting_collection() const noexcept { return this->_game_settings; };
         constexpr game_setting_collection& get_game_setting_collection() noexcept { return this->_game_settings; };

         constexpr const tes::file* get_active_file() const noexcept { return this->_files.active; }
         constexpr tes::file* get_active_file() noexcept { return this->_files.active; }

         constexpr game get_current_game() const noexcept { return this->_game; }

         constexpr const std::filesystem::path& get_data_directory() const noexcept { return this->_data_directory; }
         constexpr void set_data_directory(const std::filesystem::path& v) {
            this->_data_directory = v;
         }
         constexpr void set_data_directory(std::filesystem::path&& v) {
            this->_data_directory = std::move(v);
         }

         // May throw `exceptions::load_order_error`
         // May throw `exceptions::file_load_error`
         // May throw `exceptions::file_parse_error`
         void load(
            tes::file_load_order&,
            std::string_view active_file_name, // must also be in the provided FLO
            const callback_sets::data_load&
         );

         #pragma region Finding files
            file_prefix file_prefix_for(const std::filesystem::path& filename) const noexcept;
            file_prefix file_prefix_for(const tes::file& file) const noexcept;

            // Returns the heavy and light indices, and other associated qualifiers, that 
            // the active file has while resident in memory and will have once it's saved.
            file_prefix expected_active_file_prefix_post_save(std::optional<bool> pretend_is_or_isnt_light = {}) const noexcept;
         
            int index_of_file(const tes::file&) const noexcept;
            int index_of_prefix(file_prefix) const noexcept;
         
            const tes::file* get_file_by_prefix(file_prefix) const noexcept;
            const tes::file* get_file_by_index(int) const noexcept;
         
            bool file_is_active(const tes::file&) const noexcept;
            bool has_file(const std::filesystem::path& filename) const noexcept;
            bool has_non_active_file(const std::filesystem::path& filename) const noexcept;
         #pragma endregion

      public: // passkeyed
         constexpr form_stub_collection& _get_form_stub_collection(lu::passkey<form_stub>) noexcept { return this->_forms; };

      protected:
         void _build_use_info();
         void _make_hardcoded_forms();
   };
}
