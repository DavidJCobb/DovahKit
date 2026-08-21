#include "./form.h"
#include <utility> // std::unreachable
#include <QCoreApplication>
#include "helpers/qt/strings.h"
#include "qt/utils/bulk_string_substitution.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "../form_type_name_to_string.h"
#include "../form_identifiers_to_string.h"

namespace editor_helpers::condition_to_string {
   static QString _placement_to_string(const dovah::form_stub& stub, const options::form_format& format) {
      if (format.include_placement == options::form_id_presence::never)
         return {};
      if (format.include_placement == options::form_id_presence::if_no_editor_id)
         if (!stub.editorID.empty())
            return {};
      if (!dovah::form_type_is_reference(stub.form_type))
         return {};

      auto* cell = stub.get_parent_form();
      if (!cell || cell->form_type != dovah::form_type::cell)
         return {};

      if (!cell->editorID.empty()) {
         return QCoreApplication::translate("form referenced in condition: format string: placement", " in '%1'", "cell has editor ID")
            .arg(QString::fromStdString(cell->editorID));
      }

      auto* world = cell->get_parent_form();
      if (world && world->form_type == dovah::form_type::worldspace) {
         int32_t x;
         int32_t y;
         if (cell->get_grid_coordinates(x, y)) {
            return QCoreApplication::translate("form referenced in condition: format string: placement", " in '%1' cell (%2, %3)", "exterior cell sans editor ID")
               .arg(QString::fromStdString(world->editorID))
               .arg(x)
               .arg(y);
         }
      }

      if (format.form_type == options::form_type_format::signature) {
         return QCoreApplication::translate("form referenced in condition: format string: placement", " in [CELL:%1]", "cell sans editor ID (signature)")
            .arg(form_id_to_string(cell->formID));
      }
      return QCoreApplication::translate("form referenced in condition: format string: placement", " in cell %1", "cell sans editor ID")
         .arg(form_id_to_string(cell->formID));
   }

   extern QString form(const dovah::form_stub* stub, const options::form_format& format) {
      if (!stub || stub->is_none_stub())
         return QCoreApplication::translate("form referenced in condition", "NONE", "no form present");

      const bool is_ref = dovah::form_type_is_reference(stub->form_type);

      bool    form_lacks_editor_id = false;
      QString editor_id;
      switch (format.include_editor_id) {
         using enum options::editor_id_presence;
         case never:
            form_lacks_editor_id = stub->editorID.empty();
            break;
         case only_for_form:
            editor_id            = QString::fromStdString(stub->editorID);
            form_lacks_editor_id = editor_id.isEmpty();
            break;
         case form_or_base:
            editor_id            = QString::fromStdString(stub->editorID);
            form_lacks_editor_id = editor_id.isEmpty();
            if (form_lacks_editor_id && is_ref) {
               auto* base = dovah::form_stub_helpers::get_base_form(*stub);
               if (base) {
                  editor_id = QString::fromStdString(base->editorID);
               }
            }
            break;
      }

      QString form_id;
      switch (format.include_form_id) {
         using enum options::form_id_presence;
         case never:
            break;
         case if_no_editor_id:
         default:
            if (!form_lacks_editor_id)
               break;
            [[fallthrough]];
         case always:
            form_id = form_id_to_string(stub->formID);
            break;
      }

      QString placement = _placement_to_string(*stub, format);

      QString format_string;
      //
      // Params:
      // 
      //    %1 = Form type
      //    %2 = Editor ID
      //    %3 = Form ID
      //    %4 = Placement
      // 
      // Formats:
      // 
      //    0 = No editor ID + no form ID
      //    1 = Editor ID    + no form ID
      //    2 = No editor ID + form ID
      //    3 = Editor ID    + form ID
      //
      QString type;
      size_t  index = 0 | ((int)!editor_id.isEmpty()) | ((int)!form_id.isEmpty() << 1);
      switch (format.form_type) {
         using enum options::form_type_format;
         case none:
         default:
            format_string = std::array{
               QCoreApplication::translate("form referenced in condition: format string", "%4",        "placement only"),
               QCoreApplication::translate("form referenced in condition: format string", "%2%4",      "editor ID"),
               QCoreApplication::translate("form referenced in condition: format string", "(%3)%4",    "form ID"),
               QCoreApplication::translate("form referenced in condition: format string", "%2 (%3)%4", "editor ID + form ID"),
            }[index];
            break;
         case name:
            type          = form_type_name_to_string(stub->form_type);
            format_string = std::array{
               QCoreApplication::translate("form referenced in condition: format string", "%1%4",            "typename"),
               QCoreApplication::translate("form referenced in condition: format string", "%1: '%2'%4",      "typename + editor ID"),
               QCoreApplication::translate("form referenced in condition: format string", "%1: (%3)%4",      "typename + form ID"),
               QCoreApplication::translate("form referenced in condition: format string", "%1: '%2' (%3)%4", "typename + editor ID + form ID"),
            }[index];
            break;
         case signature:
            type          = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->form_type).signature);;
            format_string = std::array{
               QCoreApplication::translate("form referenced in condition: format string", "[%1]%4",      "signature"),
               QCoreApplication::translate("form referenced in condition: format string", "[%1]%2%4",    "signature + editor ID"),
               QCoreApplication::translate("form referenced in condition: format string", "[%1:%3]%4",   "signature + form ID"),
               QCoreApplication::translate("form referenced in condition: format string", "[%1:%3]%2%4", "signature + editor ID + form ID"),
            }[index];
            break;
      }

      auto substitution = dovahkit::qt::utils::bulk_string_substitution(format_string);
      return substitution.exec(type, editor_id, form_id, placement);
   }
}
