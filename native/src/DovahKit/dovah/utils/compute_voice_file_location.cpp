#include "./compute_voice_file_location.h"
#include <format>

namespace dovah {
   extern std::string compute_voice_filename(
      std::string_view quest_editor_id,
      std::string_view topic_editor_id,
      uint32_t info_form_id,  // load order prefix will be shaved off
      uint8_t  response_index // should be greater than 0 for correct behavior
   ) {
      if (response_index <= 0)
         return "New Response";

      std::string out;
      {
         constexpr const size_t max_length_for_quest_and_topic_ids = 25;
         constexpr const size_t max_length_for_quest_id            = 10;
         //
         // The game caps the lengths of the quest and topic editor IDs as represented in 
         // the file path.
         //
         size_t quest = quest_editor_id.size();
         size_t topic = topic_editor_id.size();
         size_t total = quest + topic;
         if (total > max_length_for_quest_and_topic_ids) {
            if (quest > max_length_for_quest_id) {
               quest_editor_id = quest_editor_id.substr(0, max_length_for_quest_id);
               quest = quest_editor_id.size();
            }
            topic_editor_id = topic_editor_id.substr(0, max_length_for_quest_and_topic_ids - quest);
         }
      }
      out += std::format("{}_{}", quest_editor_id, topic_editor_id);

      out += std::format("_{:08X}_{}", info_form_id & 0x00FFFFFF, response_index);
      return out;
   }

   extern std::string compute_voice_file_location(
      std::string_view data_filename,
      std::string_view voicetype_editor_id,
      std::string_view quest_editor_id,
      std::string_view topic_editor_id,
      uint32_t info_form_id,
      uint8_t  response_index,
      std::string_view file_extension
   ) {
      if (data_filename.empty() || voicetype_editor_id.empty())
         return {};

      std::string out = "Data\\Sound\\Voice\\";
      out += data_filename;
      out += '\\';
      out += voicetype_editor_id;
      out += '\\';
      out += compute_voice_filename(quest_editor_id, topic_editor_id, info_form_id, response_index);
      out += '.';
      out += file_extension;

      return out;
   }
}