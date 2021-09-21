#pragma once
#include <vector>
#include <QString>

namespace dovahscript {
   struct pending_script {
      QString filename;
      QString contents;
   };

   struct script_set {
      QString package_folder_name; // full path
      std::vector<pending_script> files;
   };
}