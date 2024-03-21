#pragma once
#include <string_view>
#include <vector>
#include <QMimeData>

namespace dovah {
   class form_stub;
}

namespace editor_helpers {
   constexpr const char* const form_stub_array_mime_type  = "application/dovah-kit.form-id-array";
   constexpr const char* const single_form_stub_mime_type = "application/dovah-kit.form-id";

   extern dovah::form_stub* single_form_stub_from_mime_data(const QMimeData&);
   extern std::vector<dovah::form_stub*> form_stubs_from_mime_data(const QMimeData&);

   extern void add_form_stubs_to_mime_data(QMimeData&, const std::vector<dovah::form_stub*>& list);
}