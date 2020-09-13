#pragma once
#include <QWidget>

namespace dovah {
   class form_stub;
}

void open_window_for_form(dovah::form_stub*, QWidget* parent = nullptr);