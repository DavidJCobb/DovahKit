#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>
#include "lu/passkey.h"
#include "./form_stub_collection.h"
#include "./game.h"
namespace dovah {
   namespace tes {
      class file;
   }
   struct file_prefix;
   class  form_stub;
}

namespace dovah {
   class active_load_order {
      public:
         active_load_order(const active_load_order&) = delete;
         active_load_order& operator=(const active_load_order&) = delete;

      protected:
         struct {
            std::vector<tes::file*> list; // owned
            tes::file*              active = nullptr; // unowned; also in `list`
         } _files;
         form_stub_collection _forms;
         game                 _game = game::skyrim_classic;

      public:
         constexpr const tes::file* get_active_file() const noexcept { return this->_files.active; }
         constexpr tes::file* get_active_file() noexcept { return this->_files.active; }
         constexpr game get_current_game() const noexcept { return this->_game; }
         constexpr const form_stub_collection& get_form_stub_collection() const noexcept { return this->_forms; }

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
         void _make_hardcoded_forms();
   };
}
