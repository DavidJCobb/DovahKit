#include "./update_active_file_seq_file.h"
#include <bit>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string_view>
#include <system_error>
#include <vector>
#include "dovah/files/tes_file_reading/file_loader.h"
#include "dovah/forms/Quest.h"
#include "editor/core.h"

namespace editor_helpers {
   extern update_active_file_seq_file_result update_active_file_seq_file(std::filesystem::path name_to_use_if_nameless, bool delete_if_empty) {
      assert(!name_to_use_if_nameless.empty());
      name_to_use_if_nameless = name_to_use_if_nameless.filename();

      auto& editor = DovahKitCore::get();
      assert(editor.has_data());
      assert(editor.has_active_file());
      auto* flo = editor.get_file_load_order();
      assert(!!flo);
      auto* active_file = flo->get_active_file();
      assert(!!active_file);

      std::vector<uint32_t> record_ids;
      editor.for_each_form_of_type(dovah::form_type::quest, [active_file, flo, &record_ids](dovah::form_stub* stub) -> bool {
         if (!stub->is_edited_or_in_active_file())
            return false;
         auto form_ptr = stub->load().ptr_cast<dovah::loaded_forms::Quest>();
         if (!form_ptr)
            return false;
         if (form_ptr->flags & dovah::loaded_forms::Quest::quest_flag::start_game_enabled) {
            auto record_id = flo->remap_formID_for_save(stub->formID);
            record_ids.push_back(record_id);
         }
         return false;
      });

      auto filename = active_file->get_filename();
      if (filename.empty())
         filename = name_to_use_if_nameless.string();
      auto path = std::filesystem::path(flo->base_path) / std::string_view("Seq");
      std::filesystem::create_directories(path);
      path /= filename;
      path.replace_extension("seq");

      auto temp_path = path;
      temp_path.replace_extension("dovahkit-tmp-seq");

      if (record_ids.empty()) {
         if (delete_if_empty) {
            std::error_code ec;
            std::filesystem::remove(path, ec);
            if (ec)
               return update_active_file_seq_file_result::unable_to_delete;
            return update_active_file_seq_file_result::success;
         }
      }
      std::basic_ofstream<uint8_t> stream;
      stream.open(temp_path.string().c_str(), std::ios_base::binary | std::ios_base::trunc);
      if (stream.fail())
         return update_active_file_seq_file_result::unable_to_write_temporary;
      if constexpr (std::endian::native == std::endian::little) {
         stream.write((const uint8_t*)record_ids.data(), record_ids.size() * sizeof(uint32_t));
      } else {
         for (auto id : record_ids) {
            id = std::byteswap(id);
            stream.write((const uint8_t*)&id, sizeof(uint32_t));
         }
      }
      stream.close();

      std::error_code ec;
      std::filesystem::rename(temp_path, path, ec);
      if (ec)
         return update_active_file_seq_file_result::unable_to_relocate;
      return update_active_file_seq_file_result::success;
   }
}