#include "list_none_stubs.h"
#include <QMessageBox>
#include <QString>
#include "../../../dovah/form_stub.h"
#include "../../../editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void list_none_stubs::execute(QWidget* window) {
      QString text;
      DovahKitCore::get().for_each_form_of_type(dovah::form_type::none, [&text](dovah::form_stub* stub) {
         if (!stub->is_none_stub())
            return false;
         QString entry = QString("%1 with %2 users").arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')));
         QString list;
         size_t  count = 0;
         for (auto& pair : stub->inbound) {
            ++count;
            if (!list.isEmpty())
               list += QString(", ");
            list += QString("%1").arg(pair.first, 8, 16, QChar('0'));
         }
         entry = entry.arg(count);
         if (!list.isEmpty()) {
            entry += ": ";
            entry += list;
         }
         if (!text.isEmpty())
            text += '\n';
         text += entry;
         return false;
      });
      if (text.isEmpty())
         text = "No dangling references (\"none-stubs\") found.";
      QMessageBox::information(window,
         QObject::tr("Report"),
         text,
         QMessageBox::Ok
      );
   }
}