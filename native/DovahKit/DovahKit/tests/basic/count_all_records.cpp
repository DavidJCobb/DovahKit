#include "./count_all_records.h"
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <QInputDialog>
#include "dovah/tes/file.h"
#include "dovah/tes/reading/file_view.h"

namespace dovahkit::tests {
   extern void count_all_records() {
      bool ok = false;
      QString data = QInputDialog::getText(
         nullptr,
         "Enter SSE data directory",
         "",
         {},
         "",
         &ok
      );
      if (!ok)
         return;

      std::filesystem::path data_directory = data.toStdString();
      std::filesystem::path file_path = data_directory / "Skyrim.esm";

      dovah::tes::file file(dovah::game::skyrim_special);
      try {
         file.open(file_path);
      #pragma warning(suppress: 4101)
      } catch (std::exception& ex) {
         __debugbreak();
      } catch (...) {
         __debugbreak();
      }

      dovah::tes::file_view view;
      view.set_target_file(&file);

      struct record_type_stats {
         size_t record_count = 0;
         struct {
            size_t average = 0;
            size_t max     = 0;
            size_t min     = (size_t)-1;
         } subrecord_counts;
      };

      std::unordered_map<lu::four_cc, record_type_stats> stats_by_record_type;
      do {
         auto type = view.next_record_or_group();
         if (type == dovah::tes::file_view::chunk_type::none)
            break;
         if (type != dovah::tes::file_view::chunk_type::record)
            continue;

         auto& record = view.subviews.record;
         auto& stats  = stats_by_record_type[record.signature()];
         ++stats.record_count;

         size_t subrecord_count = 0;
         while (auto& subrecord = record.next_subrecord()) {
            ++stats.subrecord_counts.average;
            ++subrecord_count;
         }
         if (subrecord_count < stats.subrecord_counts.min)
            stats.subrecord_counts.min = subrecord_count;
         if (subrecord_count > stats.subrecord_counts.max)
            stats.subrecord_counts.max = subrecord_count;
      } while (true);
      for (auto& pair : stats_by_record_type) {
         pair.second.subrecord_counts.average /= pair.second.record_count;
      }

      __debugbreak();
   }
}