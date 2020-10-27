#include "log_window.h"
#include "../../helpers/qt/strings.h"
#include "../../editor/core.h"
#include "../../dovah/files/file_read_warning.h"
#include "../../dovah/notice_code_list.h"

LogWindow::LogWindow(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   this->ui.list->setAlternatingRowColors(true);
   this->ui.list->setUniformItemSizes(false);
   this->ui.list->setWordWrap(true);
}

void LogWindow::insertLogEntry(const QString& text) {
   //this->ui.list->addItem(text);
}