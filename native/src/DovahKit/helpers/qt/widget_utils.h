#pragma once
#include <QWidget>

namespace cobb::qt {
   extern bool is_explicitly_shown(const QWidget*);
   extern bool is_explicitly_hidden(const QWidget*);
}
