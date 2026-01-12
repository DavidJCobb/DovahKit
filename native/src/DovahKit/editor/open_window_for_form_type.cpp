#include "./open_window_for_form_type.h"
#include "./subsystems/per_form_windows/core.h"

extern void open_edit_dialog_for_form_type(dovah::form_type type) {
   auto& pfwins = dovahkit::subsystems::per_form_windows::core::get_or_create();
   pfwins.show_form_type_edit_dialog(type);
}