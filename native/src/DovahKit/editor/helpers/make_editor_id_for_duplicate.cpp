#include "make_editor_id_for_duplicate.h"
#include <QRegExp>

namespace editor_helpers {
   QString make_editor_id_for_duplicate(QString editorID) {
      if (editorID.endsWith("DUPLICATE")) {
         editorID += QString("001");
      } else {
         auto re = QRegExp("DUPLICATE(\\d+)$");
         if (re.indexIn(editorID) == -1) {
            editorID += QString("DUPLICATE");
         } else {
            auto index = re.cap(1);
            bool is_int = false;
            auto val = index.toInt(&is_int) + 1;
            if (is_int) {
               editorID.chop(index.size());
               editorID += QString("%1").arg(val, index.size(), 10, QChar('0'));
            } else {
               editorID += QString("DUPLICATE");
            }
         }
      }
      return editorID;
   }
}