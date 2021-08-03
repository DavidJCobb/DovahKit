#pragma once
#include <vector>
#include <QString>

namespace dovahscript {
   struct pending_script {
      QString filename;
      QString contents;
   };

   struct script_set {
      QString addon_name;
      std::vector<pending_script> files;
   };
}