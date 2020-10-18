#include "main_window.h"
#include <QCloseEvent>
#include <QMDISubWindow>
#include <QMessageBox>
#include <QShowEvent>
#include <QtWinExtras/QWinTaskbarProgress.h> // this probably isn't the right way to include this, but Visual Studio and Qt Tools are not being cooperative.
#include "../editor/core.h"
#include "../dovah/files/file_load_order.h"
#include "main_window/load_window.h"
#include "main_window/save_window.h"
#include "main_window/file_metadata_window.h"

#include "../dovah/files/common.h"
#include "../dovah/form_stub.h"

#include <QFileDialog>
#include <QInputDialog>
#include "main_window/_debug_hooks/enumerate_bsa_contents.h"
#include "main_window/_debug_hooks/extract_bsa_file.h"
#include "main_window/_debug_hooks/lookup_bsa_file_from_bsa_load_order.h"

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
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      this->ui.actionEditFileMetadata->setDisabled(false);
      this->ui.actionSave->setDisabled(false);
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->ui.actionEditFileMetadata->setDisabled(true);
      this->ui.actionSave->setDisabled(true);
   });
   //
   this->object_window = new ObjectWindow(this);
   this->ui.mdi->addSubWindow(this->object_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
   //
   this->cell_view_window = new CellViewWindow(this);
   this->ui.mdi->addSubWindow(this->cell_view_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
   //
   this->ui.actionEditFileMetadata->setDisabled(true);
   this->ui.actionSave->setDisabled(true);
   //
   this->ui.actionLoadDataClassic->setData((int)dovah::game::skyrim_classic);
   this->ui.actionLoadDataSpecial->setData((int)dovah::game::skyrim_special);
   for (auto* action : this->ui.menuLoadData->actions()) {
      QObject::connect(action, &QAction::triggered, this, [action, this]() {
         auto modal = new LoadOrderOpenDialog((dovah::game)action->data().toInt(), this);
         modal->setModal(true);
         modal->open();
      });
   }
   //
   QObject::connect(this->ui.actionEditFileMetadata, &QAction::triggered, this, [this]() {
      if (auto dialog = this->metadata_window) {
         dialog->raise();
         dialog->activateWindow();
         return;
      }
      auto& editor = DovahKitCore::get();
      auto  dialog = new FileMetadataWindow(this);
      this->metadata_window = dialog;
      QObject::connect(dialog, &QDialog::finished, &editor, [this, dialog]() {
         this->metadata_window = nullptr;
         dialog->deleteLater();
      });
      dialog->show();
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
      auto modal = new ActiveFileSaveDialog(this);
      modal->setModal(true);
      modal->exec();
   });
   #pragma region Menu items to set editor encoding
      this->ui.actionSetEncodingWin1250->setData("Windows-1250");
      this->ui.actionSetEncodingWin1251->setData("Windows-1251");
      this->ui.actionSetEncodingWin1252->setData("Windows-1252");
      this->ui.actionSetEncodingWin1253->setData("Windows-1253");
      this->ui.actionSetEncodingWin1254->setData("Windows-1254");
      this->ui.actionSetEncodingWin1256->setData("Windows-1256");
      this->ui.actionSetEncodingUTF8->setData("UTF-8");
      for (auto* action : this->ui.menuTextEncoding->actions()) {
         action->setCheckable(true);
         action->setChecked(false);
         QObject::connect(action, &QAction::triggered, this, [action]() {
            DovahKitCore::get().set_encoding(action->data().toString().toStdString());
         });
      }
      QObject::connect(this->ui.menuTextEncoding, &QMenu::aboutToShow, this, [this]() {
         auto* menu     = this->ui.menuTextEncoding;
         auto& editor   = DovahKitCore::get();
         auto  encoding = QString::fromStdString(editor.get_encoding());
         //
         for (auto* action : menu->actions()) {
            QString n = action->data().toString();
            action->setChecked(n == encoding);
         }
      });
   #pragma endregion

   #pragma region Debugging
   QObject::connect(this->ui.actionDebugGetRecordSizeStats, &QAction::triggered, this, [this]() {
      struct deets {
         uint32_t smallest = std::numeric_limits<uint32_t>::max();
         uint32_t largest  = 0;
         uint32_t total    = 0;
         uint32_t count    = 0;
         inline float average() const noexcept { return (float)total / (float)count; }
         void consume(uint32_t size) noexcept {
            total += size;
            ++count;
            if (size < smallest)
               smallest = size;
            if (size > largest)
               largest = size;
         }
         QString report() const noexcept {
            return QString("Range: [%1, %2]; average %3 over %4 records.").arg(smallest).arg(largest).arg(average()).arg(count);
         }
      };
      //
      deets compressed;
      deets uncompressed;
      auto& editor = DovahKitCore::get();
      editor.for_each_form([&compressed, &uncompressed](const dovah::form_stub* stub) {
         dovah::tes_file_record_header header;
         uint32_t decompressed_size;
         if (stub->fetch_record_header(header, decompressed_size)) {
            if (header.body_is_compressed()) {
               compressed.consume(decompressed_size);
            } else {
               uncompressed.consume(header.size);
            }
         }
         return false;
      });
      QMessageBox::question(this,
         tr("Report"),
         tr("Compressed records:<br/>%1<br/><br/>Uncompressed records:<br/>%2").arg(compressed.report()).arg(uncompressed.report()),
         QMessageBox::Ok
      );
   });
   QObject::connect(this->ui.actionDebugLogBSAContents, &QAction::triggered, this, [this]() {
      auto name = QFileDialog::getOpenFileName(this, tr("Select BSA file", "debug"), "", "Bethesda Softworks Archives (*.bsa *.ba2)");
      if (name.isEmpty())
         return;
      DovahKitDebug::enumerate_bsa_contents(name.toStdString());
   });
   QObject::connect(this->ui.actionDebugExtractBSAFile, &QAction::triggered, this, [this]() {
      auto name = QFileDialog::getOpenFileName(this, tr("Select BSA file", "debug"), "", "Bethesda Softworks Archives (*.bsa *.ba2)");
      if (name.isEmpty())
         return;
      auto entry = QInputDialog::getText(this, tr("Path of file to extract? Do not specify a Data prefix.", "debug"), tr("Path:"));
      if (entry.isEmpty())
         return;
      auto to = QFileDialog::getSaveFileName(this, tr("Select target file", "debug"));
      if (to.isEmpty())
         return;
      DovahKitDebug::extract_bsa_file(name.toStdString(), entry.toStdString(), to.toStdString());
   });
   QObject::connect(this->ui.actionDebugLookupBSAFile, &QAction::triggered, this, [this]() {
      DovahKitDebug::lookup_bsa_file_from_bsa_load_order(this);
   });
   #pragma endregion
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
   //
   if (auto tb = this->taskbar_button)
      tb->setWindow(this->windowHandle());
   for (auto* subwindow : this->ui.mdi->subWindowList()) { // set initial object window height
      if (subwindow->widget() == this->object_window) {
         auto g_canvas = this->ui.mdi->geometry();
         auto g_subwin = subwindow->geometry();
         subwindow->move(0, 0);
         subwindow->resize(g_subwin.width(), g_canvas.height() - 2); // not sure why this is off by 2px or whether that's consistent :(
         break;
      }
   }
   //
   if (event->spontaneous()) // spontaneous events occur just after the window is visible; internal events, just before.
      emit shown();
}