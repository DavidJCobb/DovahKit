#pragma once
#include <QWidget>

namespace dovah {
   class form_stub;
}

//
// Functions in this file open form-specific windows while only allowing one such 
// window per form to be open at a time, i.e. they prevent you from having five of 
// the same window open for the same form.
//

void open_use_info_dialog_for_form(dovah::form_stub*, QWidget* parent = nullptr);
void open_edit_dialog_for_form(dovah::form_stub*, QWidget* parent = nullptr);