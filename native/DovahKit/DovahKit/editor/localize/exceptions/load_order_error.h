#pragma once
#include <QString>
namespace dovah::exceptions {
   class load_order_error;
}

namespace dovahkit::localize::exceptions {
   extern QString load_order_error(const dovah::exceptions::load_order_error&);
}