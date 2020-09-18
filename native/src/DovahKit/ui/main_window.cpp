#include "main_window.h"
#include <QCloseEvent>
#include <QShowEvent>
#include <QtWinExtras/QWinTaskbarProgress.h> // this probably isn't the right way to include this, but Visual Studio and Qt Tools are not being cooperative.
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
   this->taskbar_button = new QWinTaskbarButton(this);
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

void MainWindow::setProgressBounds(int min, int max) {
   if (!this->taskbar_button)
      return;
   auto p = this->taskbar_button->progress();
   p->setRange(min, max);
}
void MainWindow::setProgressStep(int s) {
   if (!this->taskbar_button)
      return;
   auto p = this->taskbar_button->progress();
   p->setValue(s);
}
void MainWindow::setProgressEnableState(bool s) {
   if (!this->taskbar_button)
      return;
   auto p = this->taskbar_button->progress();
   p->setVisible(s);
}

void MainWindow::closeEvent(QCloseEvent* event) {
   //
   // TODO: If the user has unsaved changes, show a confirmation prompt. If they 
   // choose not to exit, then we want to call {event->ignore()} and then return 
   // immediately.
   //
   _window = nullptr;
   event->accept();
}
void MainWindow::showEvent(QShowEvent* event) {
   event->accept();
   if (auto tb = this->taskbar_button)
      tb->setWindow(this->windowHandle());
   if (event->spontaneous()) // spontaneous events occur just after the window is visible; internal events, just before.
      emit shown();
}