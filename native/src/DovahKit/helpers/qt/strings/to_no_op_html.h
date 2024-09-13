#pragma once
#include <QString>

namespace cobb::qt::strings {
   // Given a string consisting of plaintext, convert it to a Qt HTML string that 
   // doesn't actually have any formatting. This is useful for when you need a 
   // widget (or a QAbstractItemModel data item) to have a plaintext tooltip, but 
   // you also need that tooltip to word-wrap.
   //
   // Refs:
   // https://stackoverflow.com/a/46212292
   // https://bugreports.qt.io/browse/QTBUG-41051
   //
   extern QString to_no_op_html(QString, std::string_view wrap_in_tag);
}
