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

      struct version {
         uint32_t major = 0;
         uint32_t minor = 0;
         uint32_t patch = 0;
         uint32_t build = 0;

         inline operator bool() const noexcept { return (this->major | this->minor | this->patch | this->build) != 0; }
         std::strong_ordering operator<=>(const version&) const = default;
      };

      QDir root_folder;
      //
      QString name;
      QVector<author> authors;
      QString description;
      struct {
         version package;       // version of the package itself
         version dovah_minimum; // minimum supported DovahKit version
         version dovah_tested;  // most recent DovahKit version this package was tested with
      } version_info;
      //
      dovahscript::permission_set permissions;
      QVector<QString> files;
   };
}