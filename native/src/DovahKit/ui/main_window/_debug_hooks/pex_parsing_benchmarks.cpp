#include "pex_parsing_benchmarks.h"
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "helpers/performance.h"

#include "dovah/files/bsa/bsa_archived_file.h"
#include "editor/subsystems/assets.h"

#include "dovah/files/pex/parsers/full.h"
#include "dovah/files/pex/parsers/class_info_collector.h"

namespace {
   constexpr const size_t test_count = 50;

   constexpr const std::array<const char*, 5> test_script_paths = {
      "scripts/cwthreattriggerscript.pex",
      "scripts/AlftandBlackreachLockScript.pex",
      "scripts/BFBPuzzle01Pillar.pex",
      "scripts/C03QuestScript.pex",
      "scripts/CWCampaignScript.pex",
   };

   static_assert([]() {
      for (auto* p : test_script_paths)
         if (!p || !p[0])
            return false;
      return true;
   }(), "No blank/nullptr test paths allowed.");
}

namespace DovahKitDebug::features {
   /*static*/ void pex_parsing_benchmarks::execute(QWidget* window) {
      auto& assets = dovahkit::subsystems::assets::get();

      std::array<std::string_view, test_script_paths.size()>          scriptnames = {};
      std::array<dovah::bsa_archived_file*, test_script_paths.size()> files       = {};
      for (size_t i = 0; i < test_script_paths.size(); ++i) {auto& scriptname = scriptnames[i];

         scriptname = test_script_paths[i];
         {
            auto i = scriptname.find_last_of('/');
            auto j = scriptname.find_last_of('\\');
            if (j != std::string::npos && j > i)
               i = j;
            if (i != std::string::npos)
               scriptname = scriptname.substr(i + 1);

            auto k = scriptname.find_last_of('.');
            if (k != std::string::npos)
               scriptname = scriptname.substr(0, k);
         }
      }
      for (size_t i = 0; i < test_script_paths.size(); ++i) {
         files[i] = assets.lookup_game_asset(std::filesystem::path((const char8_t*)test_script_paths[i]));
      }

      struct test_results {
         cobb::benchmark all;
         uint32_t min = 0xFFFFFFFF;
         uint32_t max = 0;
         uint32_t avg = 0;

         inline void begin() {
            all.begin();
         }
         inline void consume_single(const cobb::benchmark& single) {
            auto us = single.microseconds();
            avg += us;
            if (us < min)
               min = us;
            if (us > max)
               max = us;
         }
         inline void end() {
            all.end();
            avg /= test_count;
         }
      };

      test_results results_full;
      {
         results_full.begin();
         for (size_t i = 0; i < test_count; ++i) {
            cobb::benchmark single;

            single.begin();
            //
            for (auto* file : files) {
               if (!file)
                  continue;
               dovah::pex::parsers::full parser;
               try {
                  parser.read_file((const uint8_t*)file->data(), file->size());
               } catch (dovah::pex::exceptions::base_read_exception& e) {}
            }
            //
            single.end();

            results_full.consume_single(single);
         }
         results_full.end();
      }

      qDebug("Full-parse benchmark complete.");
      qDebug("Total time for %d parses of %d files: %d microseconds", test_count, test_script_paths.size(), results_full.all.microseconds());
      qDebug("Min: %d microseconds", results_full.min);
      qDebug("Avg: %d microseconds", results_full.avg);
      qDebug("Max: %d microseconds", results_full.max);

      test_results results_skim;
      {
         using parser_type       = dovah::pex::parsers::class_info_collector;
         using string_table_type = parser_type::shared_string_table_type;

         results_skim.begin();
         for (size_t i = 0; i < test_count; ++i) {
            cobb::benchmark single;

            string_table_type all_strings;
            single.begin();
            //
            for (size_t i = 0; i < files.size(); ++i) {
               auto* file = files[i];
               if (!file)
                  continue;
               parser_type parser(all_strings);
               parser.desired_classname = scriptnames[i];
               try {
                  parser.read_file((const char*)file->data(), file->size());
               } catch (dovah::pex::exceptions::base_read_exception& e) {}
            }
            //
            single.end();

            results_skim.consume_single(single);
         }
         results_skim.end();
      }

      qDebug("Skim-parse benchmark complete.");
      qDebug("Total time for %d parses of %d files: %d microseconds", test_count, test_script_paths.size(), results_skim.all.microseconds());
      qDebug("Min: %d microseconds", results_skim.min);
      qDebug("Avg: %d microseconds", results_skim.avg);
      qDebug("Max: %d microseconds", results_skim.max);
   }
}