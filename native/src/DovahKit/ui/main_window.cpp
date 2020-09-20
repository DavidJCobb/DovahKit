#include "main_window.h"
#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
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
   QObject::connect(this->ui.actionSave, &QAction::triggered, this, [this]() {
      auto& editor = DovahKitCore::get();
      if (!editor.has_active_file()) {
         QMessageBox::question(this,
            tr("Error", "saving"),
            tr("You did not select an active file, and there are too many files in the load order to create an active file."),
            QMessageBox::Ok
         );
         return;
      }
      //
      auto choice = QMessageBox::question(this,
         tr("Confirm", "save confirmation"),
         tr("Currently this is experimental and will only write a test file. Please confirm your understanding.", "save confirmation"),
         QMessageBox::Yes | QMessageBox::No
      );
      if (choice == QMessageBox::No)
         return;
      //
      std::filesystem::path active_file_name; // used if the active file has no name (i.e. the user chose no active file and we'd be saving a new one)
      if (!editor.active_file_has_name()) {
         //
         // TODO: a better way of asking for an active file name than this; in particular, the user 
         // should be able to see the contents of the current load order so that they know those 
         // file names are off-limits (or else we should have a custom dialog that validates that 
         // in real-time).
         //
         // A custom save dialog would also allow for checkboxes for file header flags e.g. the 
         // "ESM" flag.
         //
         bool ok;
         auto text = QInputDialog::getText(
            this,
            tr("Select filename"),
            tr("Select a name for the new file."), QLineEdit::Normal,
            tr("untitled.esp"),
            &ok
         );
         if (!ok || text.isEmpty())
            return;
         //
         // TODO: validate the filename; don't let the user type slashes or anything else weird
         //
         active_file_name = text.toStdString();
      }
      //
      editor.save_active_file(active_file_name);
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