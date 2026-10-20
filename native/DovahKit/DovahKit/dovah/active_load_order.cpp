#include "./active_load_order.h"
#include "./facts/game/game_supports_light_plugins.h"
#include "./facts/game/hardcoded_form_ids_ignore_record_id_prefix.h"
#include "./tes/file.h"
#include "./types/file_prefix.h"

namespace dovah {
   #pragma region Finding files
      file_prefix active_load_order::file_prefix_for(const std::filesystem::path& filename) const noexcept {
         uint8_t     heavy = 0xFF;   // start at -1
         uint16_t    light = 0xFFFF; // start at -1
         file_prefix prefix;
         for (auto* f : this->_files.list) {
            bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
            bool is_equal = f->path().filename() == filename;
            if (is_light) {
               ++light;
               if (is_equal)
                  prefix = file_prefix::make_light(light);
            } else {
               ++heavy;
               if (is_equal)
                  prefix = file_prefix::make_heavy(heavy);
            }
            if (is_equal) {
               if (!facts::hardcoded_form_ids_ignore_record_id_prefix(this->_game, f->header().file_version))
                  prefix.set_can_co_opt_hardcoded_range(true);
               break;
            }
         }
         return prefix;
      }
      file_prefix active_load_order::file_prefix_for(const tes::file& file) const noexcept {
         uint8_t     heavy = 0xFF;   // start at -1
         uint16_t    light = 0xFFFF; // start at -1
         file_prefix prefix;
         for (auto* f : this->_files.list) {
            bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
            bool is_equal = f == &file;
            if (is_light) {
               ++light;
               if (is_equal)
                  prefix = file_prefix::make_light(light);
            } else {
               ++heavy;
               if (is_equal)
                  prefix = file_prefix::make_heavy(heavy);
            }
            if (is_equal) {
               if (!facts::hardcoded_form_ids_ignore_record_id_prefix(this->_game, f->header().file_version))
                  prefix.set_can_co_opt_hardcoded_range(true);
               break;
            }
         }
         return prefix;
      }
      file_prefix active_load_order::expected_active_file_prefix_post_save(std::optional<bool> pretend_is_or_isnt_light) const noexcept {
         if (!this->_files.active)
            return {};

         bool can_co_opt_hardcoded_id_space = false;
         if (!facts::hardcoded_form_ids_always_ignore_record_id_prefix(this->_game)) {
            //
            // Even in a game where hardcoded form IDs aren't prefix-agnostic, the active file 
            // can only *use* that ID space if it has at least one master. If the active file 
            // has no masters, then its own record ID prefix is 0x00, and it still has to avoid 
            // the range [00000001, 000007FF].
            //
            if (this->_files.list.size() > 1) {
               can_co_opt_hardcoded_id_space = true;
            }
         }
         file_prefix prefix;
         {
            uint8_t  heavy = 0xFF;   // start at -1
            uint16_t light = 0xFFFF; // start at -1
            for (auto* f : this->_files.list) {
               bool is_light = facts::game_supports_light_plugins(this->_game) && f->header().flags.has_all_of<tes::file_header_flag::light>();
               bool is_equal = f == this->_files.active;
               if (is_equal && pretend_is_or_isnt_light.has_value()) {
                  is_light = pretend_is_or_isnt_light.value();
               }
               if (is_light) {
                  ++light;
                  prefix = file_prefix::make_light(light);
               } else {
                  ++heavy;
                  prefix = file_prefix::make_heavy(heavy);
               }
               if (is_equal)
                  break;
            }
         }
         prefix.set_can_co_opt_hardcoded_range(can_co_opt_hardcoded_id_space);
         return prefix;
      }
      //
      int active_load_order::index_of_file(const tes::file& f) const noexcept {
         size_t size = this->_files.list.size();
         for (size_t i = 0; i < size; ++i)
            if (this->_files.list[i] == &f)
               return i;
         return -1;
      }
      int active_load_order::index_of_prefix(file_prefix prefix) const noexcept {
         if (prefix.is_undefined())
            return -1;
         size_t size = this->_files.list.size();
         if (prefix.is_light()) {
            uint16_t target = prefix.light_prefix();
            uint16_t count  = -1;
            for (size_t i = 0; i < size; ++i) {
               auto* file = this->_files.list[i];
               if (file->header().flags.has_all_of<tes::file_header_flag::light>()) {
                  if (++count == target)
                     return i;
               }
            }
            return -1;
         }
         uint8_t target = prefix.load_prefix();
         uint8_t count  = -1;
         for (size_t i = 0; i < size; ++i) {
            auto* file = this->_files.list[i];
            if (!file->header().flags.has_all_of<tes::file_header_flag::light>()) {
               if (++count == target)
                  return i;
            }
         }
         return -1;
      }
      //
      const tes::file* active_load_order::get_file_by_prefix(file_prefix p) const noexcept {
         auto i = this->index_of_prefix(p);
         if (i < 0)
            return nullptr;
         return this->get_file_by_index(i);
      }
      const tes::file* active_load_order::get_file_by_index(int i) const noexcept {
         auto size = this->_files.list.size();
         if (i < 0) {
            i += size;
            if (i < 0)
               return nullptr;
         } else if (i >= size)
            return nullptr;
         return this->_files.list[i];
      }
      //
      bool active_load_order::file_is_active(const tes::file& file) const noexcept {
         return &file == this->_files.active;
      }
      bool active_load_order::has_file(const std::filesystem::path& filename) const noexcept {
         for (auto* f : this->_files.list)
            if (f->path().filename() == filename)
               return true;
         return false;
      }
      bool active_load_order::has_non_active_file(const std::filesystem::path& filename) const noexcept {
         if (auto* f = this->_files.active)
            if (f->path().filename() == filename)
               return false;
         return this->has_file(filename);
      }
   #pragma endregion
}