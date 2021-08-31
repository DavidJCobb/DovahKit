#pragma once
#include <stdexcept>
#include <QDir>
#include <QString>
#include "../../dovahscript/permission_set.h"

namespace script_packages {
   struct manifest {
      struct link {
         QString name;
         QString url;
      };
      struct author {
         QString name;
         QVector<link> links;
      };

      QDir root_folder;
      //
      QString name;
      QVector<author> authors;
      QString description;
      //
      dovahscript::permission_set permissions;
      QVector<QString> files;
   };
}