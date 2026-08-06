#pragma once
#include <QWidget>
namespace dovah {
   class form_stub;
}

namespace editor_helpers {
   extern void recalc_bounds(QWidget* parent_window, dovah::form_stub&);
}