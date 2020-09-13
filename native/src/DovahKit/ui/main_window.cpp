#include "main_window.h"
#include <QDebug>
#include <QErrorMessage>
#include <QFileDialog>
#include "../dovah/files/file_load_order.h"

#include <sys/timeb.h> // for benchmarks

namespace {
   static constexpr char* TEST_PLUGIN_PATH = "C:/temp/";
}

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
   QObject::connect(this->ui.actionOpen, &QAction::triggered, this, [this]() {
      //
      // TODO: file_load_order relies on having a single "base path" i.e. all files must be in the 
      // same folder. This sucks and we should change it
      //
      QString base_path = QFileDialog::getExistingDirectory(this, tr("Set base path"));
      #if !_DEBUG
         static_assert(false, "Finish me");
      #endif
   });
   QObject::connect(this->ui.actionDebugLoadSkyrim, &QAction::triggered, this, [this]() {
      dovah::file_load_order order;
      order.queued_load.base_path = TEST_PLUGIN_PATH;
      order.queue_file("Skyrim.esm");
      struct timeb bench_start;
      struct timeb bench_end;
      ftime(&bench_start);
      bool result = order.load_queued_files();
      ftime(&bench_end);
      //
      if (order.load_error.defined()) {
         qDebug() << "Failed to load Skyrim.esm.";
         qDebug() << "Time taken: " << ((uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm)) << " ms";
         //
         auto& e = order.load_error;
         QString text = QString("%1\nFile: %2\nDependency: %3\n\n%4\n\nForm ID: %5\nOffset: %6")
            .arg(e.code_string())
            .arg(e.file.c_str())
            .arg(e.dependency.c_str())
            .arg(e.message.c_str())
            .arg(e.formID)
            .arg(e.fileOffset);
         //
         auto dialog = new QErrorMessage(this);
         dialog->showMessage(text);
         return;
      }
      qDebug() << "Loaded Skyrim.esm.";
      qDebug() << "Time taken: " << ((uint32_t)(1000.0 * (bench_end.time - bench_start.time)) + (bench_end.millitm - bench_start.millitm)) << " ms";
      __debugbreak();
   });
}