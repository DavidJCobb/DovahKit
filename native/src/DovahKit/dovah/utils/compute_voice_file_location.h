#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace dovah {
   //
   // Computes the filename, but not extension, of a TopicInfo response voice line.
   //
   // NOTE: If the info you want to look up relies on a SharedInfo, then you should 
   //       instead pass the data (data filename, topic editor ID, and info form ID)
   //       of the source SharedInfo.
   //
   extern std::string compute_voice_filename(
      std::string_view quest_editor_id,
      std::string_view topic_editor_id,
      uint32_t info_form_id,  // load order prefix will be shaved off
      uint8_t  response_index // should be greater than 0 for correct behavior
   );

   //
   // Computes the path and filename of a TopicInfo response voice line.
   // 
   // The `data_filename` should be the filename of the first datafile that defines 
   // the TopicInfo.
   //
   // NOTE: If the info you want to look up relies on a SharedInfo, then you should 
   //       instead pass the data (data filename, topic editor ID, and info form ID)
   //       of the source SharedInfo.
   // 
   // NOTE: The game allocates MAX_PATH for this and will truncate paths (it uses 
   //       functions from the C standard Annex K for string manip). This function, 
   //       on the other hand, does not truncate paths.
   // 
   //       The game (and this function) cap the quest and topic editor ID lengths, 
   //       so the only possible causes of overflow are the names of the datafile 
   //       or voicetype, or the file extension.
   //
   extern std::string compute_voice_file_location(
      std::string_view data_filename,       // e.g. "MyCoolMod.esp"; cannot be empty
      std::string_view voicetype_editor_id, // cannot be empty
      std::string_view quest_editor_id,
      std::string_view topic_editor_id,
      uint32_t info_form_id,   // load order prefix will be shaved off
      uint8_t  response_index, // should be greater than 0 for correct behavior
      std::string_view file_extension // the game prefers INI setting [Voice]sFileTypeGame
   );
}