#pragma once
#include <filesystem>

namespace editor_helpers {
   enum class update_active_file_seq_file_result {
      success,
      unable_to_write_temporary,
      unable_to_delete,
      unable_to_relocate,
   };
   extern update_active_file_seq_file_result update_active_file_seq_file(std::filesystem::path name_to_use_if_nameless, bool delete_if_empty);
}