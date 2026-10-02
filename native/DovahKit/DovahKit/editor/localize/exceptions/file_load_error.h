#pragma once
#include <QString>
namespace dovah::exceptions {
   class file_load_error;
}

namespace dovahkit::localize::exceptions {
   extern QString file_load_error(const dovah::exceptions::file_load_error&);
}