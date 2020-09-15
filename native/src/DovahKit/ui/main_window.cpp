#include "main_window.h"
#include "../editor/core.h"
#include "../dovah/files/file_load_order.h"
#include "main_window/load_window.h"

namespace {
   MainWindow* _window = nullptr;
}
/*static*/ MainWindow& MainWindow::get() {
   assert(_window && "You shouldn't be calling ReachVariantTool::get before the main window is actually created!");
   return *_window;
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
   ui.setupUi(this);
   _window = this;
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::fileLoadStatisticsAvailable, [this](const DovahKitCore::file_load_stats& stats) {
      auto text = QString("Loaded all files in %1 ms.").arg(stats.milliseconds);
      this->statusBar()->showMessage(text);
   });
   //
   this->object_window = new ObjectWindow(this);
   this->ui.mdi->addSubWindow(this->object_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
   //
   QObject::connect(this->ui.actionOpen, &QAction::triggered, this, [this]() {
      auto modal = new LoadOrderOpenDialog(this);
      modal->setModal(true);
      modal->open();
   });
}