#include "assets.h"
#include "../core.h"

#include <fstream>
#include "dovah/files/bsa/bsa_archive.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/files/bsa/bsa_load_order.h"
#include "dovah/files/bsa/utils/path_to_hashes.h"
#include "helpers/memory.h" // cobb::generic_buffer

namespace {
   static std::filesystem::path get_loose_data_path() {
      auto& core = DovahKitCore::get();

      std::filesystem::path loose_path;
      if (!core.get_game_path(loose_path, core.get_current_game()))
         return {};
      loose_path /= "Data";
      return loose_path;
   }
   static bool path_is_file(const std::filesystem::path& path) {
      auto status = std::filesystem::status(path);
      return status.type() == std::filesystem::file_type::regular;
   }
}

namespace dovahkit::subsystems {
   assets::assets() {
   }

   const dovah::bsa_load_order* assets::get_bsa_load_order() {
      return DovahKitCore::get().get_bsa_load_order();
   }
   
   dovah::bsa_archived_file* assets::lookup_loose_game_asset(const std::filesystem::path& path) {
      std::filesystem::path loose_path = get_loose_data_path();
      if (loose_path.empty())
         return nullptr;
      loose_path /= path;
      //
      std::ifstream stream;
      stream.open(loose_path, std::ios_base::in | std::ios_base::binary);
      if (!stream.is_open()) {
         return nullptr;
      }
      std::error_code error;
      auto size = std::filesystem::file_size(loose_path, error);
      if ((bool)error)
         return nullptr;
      //
      cobb::generic_buffer buffer;
      buffer.resize(size);
      stream.read((std::ifstream::char_type*)buffer.data(), buffer.size());
      return new dovah::bsa_archived_file(std::move(buffer));
   }
   dovah::bsa_archived_file* assets::lookup_game_asset(const std::filesystem::path& path, bool allow_loose_files) {
      if (allow_loose_files) {
         auto* file = this->lookup_loose_game_asset(path);
         if (file)
            return file;
      }
      auto* archives = this->get_bsa_load_order();
      if (!archives)
         return nullptr;
      return archives->lookup_file(path.string(), false);
   }
   bool assets::game_asset_exists(const std::filesystem::path& path, bool allow_loose_files) {
      if (allow_loose_files) {
         std::filesystem::path loose_path = get_loose_data_path();
         if (!loose_path.empty()) {
            loose_path /= path;
            if (path_is_file(loose_path))
               return true;
         }
      }

      auto* archives = this->get_bsa_load_order();
      if (!archives)
         return false;

      const auto opt_hash_pair = dovah::bsa::utils::path_to_hashes(path.string());
      if (!opt_hash_pair.has_value())
         return false;
      auto [folder_hash, file_hash] = opt_hash_pair.value();

      const auto& archive_list = archives->get_archive_list();
      for (auto it = archive_list.rbegin(); it != archive_list.rend(); ++it) {
         const auto* archive = *it;
         if (!archive)
            continue;
         const auto* file_info = archive->lookup_file_info(folder_hash, file_hash);
         if (!file_info)
            continue;
         if (file_info->corrupt)
            continue;
         if (file_info->size() == 0)
            continue;
         return true;
      }
      return false;
   }
   dovah::compiled_papyrus_script assets::parse_compiled_script(const std::string& scriptname) {
      using out_t  = dovah::compiled_papyrus_script;
      using file_t = dovah::bsa_archived_file*;
      
      std::string path = "scripts/" + scriptname + ".pex";
      
      auto* file = this->lookup_game_asset(path, true);
      if (!file)
         return {};

      dovah::compiled_papyrus_script data;
      data.read_file(file->data(), file->size()); // NOTE: can throw exceptions
      delete file;
      return data;
   }
}