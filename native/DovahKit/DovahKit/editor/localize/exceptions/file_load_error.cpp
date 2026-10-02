#include "./file_load_error.h"
#include <QCoreApplication>
#include "dovah/exceptions/file_load_error.h"
#include "dovah/exceptions/file_load_errors/filesystem_error.h"
#include "dovah/exceptions/file_load_errors/malformed_file_header.h"
#include "../errno_code.h"

namespace {
   using base_ex_type = dovah::exceptions::file_load_error;
   namespace ex_subtypes {
      using namespace dovah::exceptions::file_load_errors;
   }
}

namespace dovahkit::localize::exceptions {
   extern QString file_load_error(const base_ex_type& ex) {
      if (auto* casted = dynamic_cast<const ex_subtypes::filesystem_error*>(&ex)) {
         return QCoreApplication::translate(
            "dovah::exceptions::file_load_errors::filesystem_error",
            "An error occurred when trying to load file %1. %2"
         )
            .arg(ex.file_path.filename().string())
            .arg(errno_code(casted->error_code))
         ;
      }
      if (auto* casted = dynamic_cast<const ex_subtypes::malformed_file_header*>(&ex)) {
         return QCoreApplication::translate(
            "dovah::exceptions::file_load_errors::malformed_file_header",
            "An error occurred when trying to load file %1. The file header contained invalid data at "
            "file offset 0x%2."
         )
            .arg(ex.file_path.filename().string())
            .arg(casted->offset, 0, 16, '0')
         ;
      }
      return QCoreApplication::translate("dovah::exceptions::file_load_error", "An unknown error occurred when trying to load file %1.")
         .arg(ex.file_path.filename().string())
      ;
   }
}