#include "./errno_code.h"
#include <QCoreApplication>
#include <errno.h>

#define STRING(s) QCoreApplication::translate("errno_t", s)

namespace dovahkit::localize {
   extern QString errno_code(int errno_code) {
      switch (errno_code) {
         case ENFILE:
            return STRING("Too many files open (system-wide).");
         case EMFILE:
            return STRING("Too many files open (this process).");
         case EINVAL:
            return STRING("The file name or path may be invalid");
         case ELOOP:
            return STRING("The file was inaccessible due to a cyclical reference among symbolic links in the file path.");
         case ENAMETOOLONG:
            return STRING("The file was inaccessible; the path name (whether before or after symbolic links) is too long.");
         case EACCES:
            return STRING("The file is locked, or you do not have permission to access it.");
         case EBUSY:
            return STRING("The file is locked.");
         case ENOENT:
            return STRING("The file does not exist.");
         case EROFS:
            return STRING("The file exists on a read-only filesystem and cannot be opened for writing.");
         case ENOMEM:
            return STRING("Insufficient memory.");
         case EISDIR:
            return STRING("The \"file\" is actually a directory and therefore cannot be opened for writing.");
         case ENOTDIR:
            return STRING("The specified path is not a directory.");
      }
      return STRING("Errno value: %1").arg(errno_code);
   }
}