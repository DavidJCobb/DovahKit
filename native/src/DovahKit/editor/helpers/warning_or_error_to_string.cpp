#include "warning_or_error_to_string.h"
#include <QObject>
#include "../../helpers/qt/strings.h"
#include "../../dovah/notice_code_list.h"

#include "../../dovah/forms/Landscape.h"

namespace {
   QString _read_error_form_id_to_string(const dovah::detailed_notice::relevant_form& form) {
      QString signature = cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(form.type).signature);
      QString local     = QObject::tr("????????", "log window - missing form ID");
      QString fixed     = QObject::tr("--------", "log window - missing form ID");
      if (form.fixedID)
         fixed = QString("%1").arg(form.fixedID, 8, 16, QChar('0')).toUpper();
      if (form.localID) {
         local = QString("%1").arg(form.localID, 8, 16, QChar('0')).toUpper();
         return QObject::tr("[%1][Local:%2][Loaded:%3]").arg(signature).arg(local).arg(fixed);
      }
      return QObject::tr("[%1:%2]").arg(signature).arg(fixed);
   }
}

namespace editor_helpers {
   extern QString warning_or_error_to_string(const dovah::detailed_notice& notice) {
      using notice_code = dovah::notice_code;
      //
      QString text;
      //
      bool non_continuable_success = false;
      switch (notice.code) {
         default:
            text = QObject::tr("Unknown error.", "write error");
            break;
      }
      return text;
   }
}