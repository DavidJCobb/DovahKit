#include "load_window.h"
#include <array>
#include <QErrorMessage>
#include "../../editor/core.h"
#include "../main_window.h"

namespace {
   static std::array<QString, 5> _official_content = {{
      "Skyrim.esm",
      "Update.esm",
      "Dawnguard.esm",
      "HearthFires.esm",
      "Dragonborn.esm",
   }};
   bool _is_official_content(const QString& filename) {
      for (auto& official : _official_content)
         if (official.compare(filename, Qt::CaseInsensitive) == 0)
            return true;
      return false;
   }
}

LoadOrderOpenDialog::LoadOrderOpenDialog(bool is_skyrim_classic, QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->ui.fileList->listFiles(is_skyrim_classic);
   //
   this->_load_poller.setSingleShot(false);
   this->_load_poller.setInterval(100); // ms
   QObject::connect(&this->_load_poller, &QTimer::timeout, this, &LoadOrderOpenDialog::loadPoll);
   //
   this->ui.dateCreated->setText("");
   this->ui.dateModified->setText("");
   QObject::connect(this->ui.fileList->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
      this->ui.author->setText("");
      this->ui.description->setPlainText("");
      this->ui.dateCreated->setText("");
      this->ui.dateModified->setText("");
      this->ui.dependencies->clear();
      //
      auto widget = this->ui.fileList;
      auto model  = (LoadOrderFileList::model_type*)widget->model();
      auto active = model->activeFile();
      auto select = widget->selectionModel()->selection().indexes();
      if (select.size() > 0) {
         auto& idx = select[0];
         const auto* item = (LoadOrderFileList::model_item_type*)idx.internalPointer();
         if (item) {
            this->ui.author->setText(item->author);
            this->ui.description->setPlainText(item->description);
            this->ui.dateCreated->setText(item->created.toString());
            this->ui.dateModified->setText(item->modified.toString());
            //
            auto& list = item->dependencies;
            auto  size = item->dependencies.size();
            for (size_t i = 0; i < size; ++i) {
               this->ui.dependencies->insertItem(i, list[i]);
            }
            //
            if (item == active) {
               this->ui.buttonSetActiveFile->setText(tr("Un-set as Active File"));
            } else {
               this->ui.buttonSetActiveFile->setText(tr("Set as Active File"));
            }
            //
            // Grey out the "set as active file" button if an official game file is selected:
            //
            bool is_official = _is_official_content(item->filename);
            this->ui.buttonSetActiveFile->setDisabled(is_official);
         }
      }
   });
   QObject::connect(this->ui.buttonSetActiveFile, &QPushButton::clicked, [this]() {
      auto widget = this->ui.fileList;
      auto model  = (LoadOrderFileList::model_type*)widget->model();
      auto select = widget->selectionModel()->selection().indexes();
      if (select.size() > 0) {
         auto& idx  = select[0];
         auto* item = (LoadOrderFileList::model_item_type*)idx.internalPointer();
         if (item) {
            //
            // Do not allow the user to set official content as the active file. (This should never 
            // be possible -- the button should be greyed out if an official file is selected -- but 
            // it never hurts to be careful.)
            //
            if (_is_official_content(item->filename)) {
               QApplication::beep(); // beep, just so the user knows that we're actively rejecting their input rather than simply not handling it
               return;
            }
            //
            if (item == model->activeFile()) {
               model->setActiveFile(nullptr);
               this->ui.buttonSetActiveFile->setText(tr("Set as Active File"));
            } else {
               model->setActiveFile(item);
               this->ui.buttonSetActiveFile->setText(tr("Un-set as Active File"));
            }
         }
      }
   });
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, [this]() {
      MainWindow::get().setProgressEnableState(false);
      //
      this->_loading = false;
      this->_load_poller.stop();
      this->accept();
   });
   QObject::connect(&editor, &DovahKitCore::dataAcquireFailed, [this](const dovah::file_read_error& e) {
      if (!this->_loading)
         return;
      this->_loading = false;
      this->_load_poller.stop();
      MainWindow::get().setProgressEnableState(false);
      //
      QString text = QString("%1<br/>File: %2<br/>Dependency: %3<br/><br/>%4<br/><br/>Form ID: %5<br/>Offset: %6")
         .arg(e.code_string())
         .arg(e.file.c_str())
         .arg(e.dependency.c_str())
         .arg(e.message.c_str())
         .arg(e.formID)
         .arg(e.fileOffset);
      //
      auto dialog = new QErrorMessage(this);
      dialog->showMessage(text);
      //
      this->reject();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, [this]() {
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, [this]() {
      this->commit();
   });
}

void LoadOrderOpenDialog::loadPoll() {
   if (!this->_loading)
      return;
   auto& main_window = MainWindow::get();
   float progress    = DovahKitCore::get().assess_load_progress();
   if (isnan(progress) || progress <= 0.00001F) {
      main_window.setProgressBounds(0, 0);
      main_window.setProgressStep(0);
   } else {
      progress *= 1000.0F;
      main_window.setProgressBounds(0, 1000);
      main_window.setProgressStep(progress);
   }
   main_window.setProgressEnableState(true);
}

void LoadOrderOpenDialog::blockUI() {
   this->ui.fileList->setDisabled(true);
   this->ui.author->setDisabled(true);
   this->ui.description->setDisabled(true);
   this->ui.dependencies->setDisabled(true);
   this->ui.buttonOK->setDisabled(true);
   this->ui.buttonCancel->setDisabled(true);
   this->ui.buttonSetActiveFile->setDisabled(true);
}
void LoadOrderOpenDialog::commit() {
   this->blockUI();
   if (this->_loading)
      return;
   this->_loading = true;
   {  // Progress indicator
      auto& main_window = MainWindow::get();
      main_window.setProgressBounds(0, 0);
      main_window.setProgressStep(0);
      main_window.setProgressEnableState(true);
   }
   //
   auto& editor = DovahKitCore::get();
   auto  model  = (LoadOrderFileList::model_type*) this->ui.fileList->model();
   //
   editor.abandon_data();
   //
   std::filesystem::path install_path;
   bool is_skyrim_classic = this->ui.fileList->isSkyrimClassic();
   editor.get_game_path(install_path, is_skyrim_classic);
   install_path.append("Data");
   editor.set_load_order_folder(install_path);
   //
   for (const auto* file : model->files()) {
      if (!file->selected)
         continue;
      std::filesystem::path path = file->filename.toStdWString();
      editor.queue_load_order_file(path);
   }
   if (const auto* file = model->activeFile()) {
      std::filesystem::path path = file->filename.toStdWString();
      editor.set_queued_active_file(path);
   }
   //
   editor.acquire_load_order_data(true);
   this->_load_poller.start();
}