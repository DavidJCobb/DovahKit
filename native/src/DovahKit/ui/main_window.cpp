#include "main_window.h"
#include <QCloseEvent>
#include <QMDISubWindow>
#include <QMessageBox>
#include <QShowEvent>
#include <QtWinExtras/QWinTaskbarProgress.h> // this probably isn't the right way to include this, but Visual Studio and Qt Tools are not being cooperative.
#include "helpers/qt/strings.h"
#include "widgets/DKStatusBar.h"
#include "editor/core.h"
#include "editor/subsystems/message_log/core.h"
#include "editor/open_window_for_form.h"
#include "editor/open_window_for_form_type.h"
#include "dovah/data/game/hardcoded_form_ids_ignore_record_id_prefix.h"
#include "dovah/data/game.h"
#include "dovah/files/file_header.h"
#include "dovah/files/file_load_order.h"
#pragma region subwindows
   #include "./main_window/cell_view.h"
   #include "./main_window/object_window.h"
   #include "./main_window/log_window.h"
   #include "./main_window/render_window.h"
#pragma endregion
#pragma region dialogs
   #include "./main_window/load_window.h"
   #include "./main_window/save_window.h"
   #include "./main_window/file_metadata_window.h"
   #include "./main_window/game_setting_window.h"
   #include "./main_window/default_object_window.h"
   #include "./main_window/script_window_single.h"
   #include "./main_window/script_window_package.h"
   //
   #include "./options_window/options_window.h"
#pragma endregion

#include "dovah/files/common.h"
#include "dovah/form_stub.h"

// For the "Windows" menu
#include "form_windows/_base.h"
#include "main_window/form_use_info.h"

#include "main_window/_debug_hooks/_setup.h"

namespace {
   using logging_subsystem = dovahkit::subsystems::message_log::core;
}

namespace {
   MainWindow* _window = nullptr;
}
/*static*/ MainWindow& MainWindow::get() {
   assert(_window && "You shouldn't be calling MainWindow::get before the main window is actually created!");
   return *_window;
}

void MainWindow::_subwindow_base::_open(QMdiArea* parent) {
   if (auto* win = this->_window) {
      if (!win->mdiArea()) { // was the subwindow removed?
         win->setWidget(this->_widget); // QMdiSubWindow may clear its widget after being closed
         parent->addSubWindow(win, this->flags);
      }
      win->show();
      {
         //
         // The implementation of QMdiSubWindow has a bug. QMdiSubWindow::close 
         // calls QWidget::close on the wrapped widget, in part so the widget 
         // can receive `closeEvent` and potentially reject the close operation. 
         // The default implementation of QWidget::close will hide the widget if 
         // the event is accepted (which, by default, it is).
         // 
         // QMdiSubWindow::showEvent doesn't *un-hide* the widget. So if you 
         // close and reopen a subwindow, it just ends up blank.
         //
         this->_widget->show(); // ...unless you do this.
      }
      win->raise();
      win->activateWindow();
      return;
   }
   QMdiSubWindow* win = this->_window = new QMdiSubWindow(parent);
   win->setWidget(this->_widget);
   win->setOption((QMdiSubWindow::SubWindowOption)(int)this->options, true);
   parent->addSubWindow(this->_window, this->flags);
   win->show(); // needed for Render Window when it's not open by default, for whatever reason
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
   ui.setupUi(this);
   _window = this;
   //
   this->taskbar_button = new QWinTaskbarButton(this);
   
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::fileLoadStatisticsAvailable, [this](const DovahKitCore::file_load_stats& stats) {
      auto text = QString("Loaded all files in %1 ms.").arg(stats.milliseconds);
      this->statusBar()->showMessage(text);
   });
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      this->ui.actionEditFileMetadata->setDisabled(false);
      this->ui.menuManageFormIDs->setDisabled(false);
      this->ui.actionSave->setDisabled(false);
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->ui.actionEditFileMetadata->setDisabled(true);
      this->ui.actionSave->setDisabled(true);
   });

   this->subwindows.object.flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint;
   this->subwindows.object.open(this->ui.mdi);
   this->subwindows.cell_view.flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint;
   this->subwindows.cell_view.open(this->ui.mdi);
   this->subwindows.log.open(this->ui.mdi);
   this->subwindows.render.flags = Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
   this->subwindows.render.options |= QMdiSubWindow::SubWindowOption::RubberBandResize;
   //this->subwindows.render.open(this->ui.mdi);
   // // don't open by default; Render Window performance currently sucks
   // // (high frame rates, but bogs down CPU. actual CPU usage isn't that 
   // // high but maybe too much CPU-to-GPU I/O by accident somewhere?)

   // Status bar
   {
      auto* status_bar = new DKStatusBar(this);
      this->setStatusBar(status_bar);

      {
         auto& info      = this->_status_bar_widgets.warning_count;
         auto* container = info.container = new QWidget(status_bar);
         auto* label     = info.label     = new QLabel(container);
         {
            auto* icon = info.icon = new QLabel(this);
            icon->setScaledContents(true);

            auto* layout = new QHBoxLayout(container);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(icon);
            layout->addWidget(label);
            status_bar->addPermanentWidget(container);
         }
         label->setText("No warnings logged");

         auto& ml = logging_subsystem::get_or_create();
         QObject::connect(&ml, &logging_subsystem::warningCountsChanged, this, &MainWindow::updateStatusBarWarningsCount);
         this->updateStatusBarWarningsCount(0, 0);
      }
   }
   
   #pragma region Window menu
   {
      QAction* action;
      QMenu*   menu = this->ui.menuWindow;
      //
      action = new QAction(menu);
      action->setText(tr("Cell View Window", "main window - window menu"));
      QObject::connect(action, &QAction::triggered, this, [this]() { this->subwindows.cell_view.open(this->ui.mdi); });
      menu->addAction(action);
      //
      action = new QAction(menu);
      action->setText(tr("Log Window", "main window - window menu"));
      QObject::connect(action, &QAction::triggered, this, [this]() { this->subwindows.log.open(this->ui.mdi); });
      menu->addAction(action);
      //
      action = new QAction(menu);
      action->setText(tr("Object Window", "main window - window menu"));
      QObject::connect(action, &QAction::triggered, this, [this]() { this->subwindows.object.open(this->ui.mdi); });
      menu->addAction(action);
      //
      action = new QAction(menu);
      action->setText(tr("Render Window", "main window - window menu"));
      QObject::connect(action, &QAction::triggered, this, [this]() { this->subwindows.render.open(this->ui.mdi); });
      menu->addAction(action);
      //
      this->form_edit_window_menu = new QMenu;
      action = menu->addMenu(this->form_edit_window_menu);
      action->setText(tr("Form dialogs", "main window - window menu"));
      //
      this->form_uses_window_menu = new QMenu;
      action = menu->addMenu(this->form_uses_window_menu);
      action->setText(tr("Use Info dialogs", "main window - window menu"));
      //
      #pragma region Form-editing window list
         QObject::connect(this->form_edit_window_menu, &QMenu::aboutToShow, this, [this]() {
            this->updateFormEditWindowList();
         });
         QObject::connect(this->form_edit_window_menu, &QMenu::triggered, this, [this](QAction* action) {
            uint32_t formID = action->data().toUInt();
            if (!formID)
               return;
            auto* form = DovahKitCore::get().get_form(formID);
            if (form)
               open_edit_dialog_for_form(*form, this);
         });
         // Don't clear menu items on aboutToHide; apparently that runs before triggered and deletes the action out from under us, ugh
         /*QObject::connect(this->form_edit_window_menu, &QMenu::aboutToHide, this, [this]() {
            this->form_edit_window_menu->clear();
         });*/
      #pragma endregion
      //
      #pragma region Use Info window list
         QObject::connect(this->form_uses_window_menu, &QMenu::aboutToShow, this, [this]() {
            this->updateFormUsesWindowList();
         });
         QObject::connect(this->form_uses_window_menu, &QMenu::triggered, this, [this](QAction* action) {
            uint32_t formID = action->data().toUInt();
            if (!formID)
               return;
            auto* form = DovahKitCore::get().get_form(formID);
            if (form)
               open_use_info_dialog_for_form(*form, this);
         });
         // Don't clear menu items on aboutToHide; apparently that runs before triggered and deletes the action out from under us, ugh
         /*QObject::connect(this->form_uses_window_menu, &QMenu::aboutToHide, this, [this]() {
            this->form_uses_window_menu->clear();
         });*/
      #pragma endregion
   }
   #pragma endregion
   
   this->ui.actionEditFileMetadata->setDisabled(true);
   this->ui.menuManageFormIDs->setDisabled(true);
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
      auto* dialog = new FileMetadataWindow(this);
      this->metadata_window = dialog;
      QObject::connect(dialog, &QDialog::finished, this, [this, dialog]() {
         this->metadata_window = nullptr;
         dialog->deleteLater();
      });
      dialog->show();
   });
   QObject::connect(this->ui.actionCameraPaths, &QAction::triggered, this, [this]() {
      open_edit_dialog_for_form_type(dovah::form_type::camera_path);
   });
   QObject::connect(this->ui.actionDefaultObjects, &QAction::triggered, this, [this]() {
      if (auto dialog = this->default_object_window) {
         dialog->raise();
         dialog->activateWindow();
         return;
      }
      auto* dialog = new DefaultObjectWindow(this);
      this->default_object_window = dialog;
      QObject::connect(dialog, &QDialog::finished, this, [this, dialog]() {
         this->default_object_window = nullptr;
         dialog->deleteLater();
      });
      dialog->show();
   });
   QObject::connect(this->ui.actionGameSettings, &QAction::triggered, this, [this]() {
      if (auto dialog = this->game_setting_window) {
         dialog->raise();
         dialog->activateWindow();
         return;
      }
      auto* dialog = new GameSettingWindow(this);
      this->game_setting_window = dialog;
      QObject::connect(dialog, &QDialog::finished, this, [this, dialog]() {
         this->game_setting_window = nullptr;
         dialog->deleteLater();
      });
      dialog->show();
   });
   QObject::connect(this->ui.actionIdleAnimations, &QAction::triggered, this, [this]() {
      open_edit_dialog_for_form_type(dovah::form_type::idle);
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

   QObject::connect(this->ui.actionCompactFormIDs, &QAction::triggered, this, [this]() {
      auto& editor = DovahKitCore::get();
      const auto current_game = editor.get_current_game();

      //
      // TODO: Make this a dialog, where the user can pick the BEES setting, with 
      // it defaulted based on the current file's attributes.
      // 
      // Additionally, the dialog should allow the user to only run the operation 
      // if the number of forms present can fit in the ESL's allowed range (BEES 
      // or no BEES).
      //

      bool allow_bees = false;
      if (current_game != dovah::game::skyrim_classic) {
         //
         // The active file can use the extended ESL form ID range if it has at least one master.
         //
         if (editor.get_loaded_files().size() > 1) {
            allow_bees = true;
            auto min_version = dovah::game_feature_support::hardcoded_form_ids_ignore_record_id_prefix_until_file_version(current_game);
            if (min_version.has_value()) { // else they always ignore them
               if (auto* header = editor.get_active_file_header()) {
                  if (header->file_version < min_version.value())
                     allow_bees = false;
               }
            }
         }
      }

      bool  success = editor.try_compact_form_ids(true, allow_bees, false);
      if (success) {
         QMessageBox::information(this,
            tr("Success", "saving"),
            tr("Form IDs have been compacted. Forms whose IDs were already in the valid range for an ESL were not moved."),
            QMessageBox::Ok
         );
      } else {
         // should be impossible
         QMessageBox::critical(this,
            tr("Error", "saving"),
            tr("Unable to compact form IDs. Please report this to DovahKit's developer, and provide a saved copy of your current file."),
            QMessageBox::Ok
         );
      }
   });
   QObject::connect(this->ui.actionMoveFormIDsOutOfBEES, &QAction::triggered, this, [this]() {
      bool success = DovahKitCore::get().try_move_form_ids_out_of_hardcoded_ambiguous_range();
      if (success) {
         QMessageBox::information(this,
            tr("Success", "saving"),
            tr("Form IDs have been moved out of the range [xxyyy000, xxyyy7FF]."),
            QMessageBox::Ok
         );
      } else {
         // should be impossible
         QMessageBox::critical(this,
            tr("Error", "saving"),
            tr("Unable to move form IDs out of the range [xxyyy000, xxyyy7FF]. Please report this to DovahKit's developer, and provide a saved copy of your current file."),
            QMessageBox::Ok
         );
      }
   });
      
   QObject::connect(this->ui.actionRunScriptSingle, &QAction::triggered, this, [this]() {
      auto* modal = new EditorSingleScriptWindow(this);
      QObject::connect(modal, &EditorSingleScriptWindow::closed, this, [modal]() {
         modal->deleteLater();
      });
      modal->show();
      modal->setWindowModality(Qt::ApplicationModal);
   });
   QObject::connect(this->ui.actionRunScriptPackage, &QAction::triggered, this, [this]() {
      auto* modal = new EditorScriptPackageWindow(this);
      modal->setModal(true);
      modal->exec();
      modal->deleteLater();
   });

   QObject::connect(this->ui.actionOptions, &QAction::triggered, this, [this]() {
      auto* win = OptionsWindow::open(this);
      QObject::connect(win, &QDialog::finished, win, &QObject::deleteLater);
   });

   DovahKitDebug::add_features_to_menu(this->ui.menuDebug);
}
MainWindow::~MainWindow() {
   _window = nullptr;
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

QMdiSubWindow* MainWindow::getSubwindowFor(QWidget* w) const noexcept {
   auto windows = this->ui.mdi->subWindowList();
   for (auto* win : windows)
      if (win->widget() == w)
         return win;
   return nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event) {
   //
   // TODO: If the user has unsaved changes, show a confirmation prompt. If they 
   // choose not to exit, then we want to call {event->ignore()} and then return 
   // immediately.
   //
   event->accept();
}
void MainWindow::showEvent(QShowEvent* event) {
   event->accept();
   //
   if (auto tb = this->taskbar_button)
      tb->setWindow(this->windowHandle());
   //
   auto g_canvas = this->ui.mdi->geometry();
   if (auto* subwindow = this->subwindows.object._window) { // set initial object window height
      auto g_subwin = subwindow->geometry();
      subwindow->move(0, 0);
      subwindow->resize(g_subwin.width(), g_canvas.height() - 2); // not sure why this is off by 2px or whether that's consistent :(
   }
   //
   if (event->spontaneous()) // spontaneous events occur just after the window is visible; internal events, just before.
      emit shown();
}

void MainWindow::updateFormEditWindowList() {
   this->form_edit_window_menu->clear();
   //
   auto& editor = DovahKitCore::get();
   editor.for_each_form_edit_dialog([this](FormEditDialogInterface* dialog) {
      auto* stub = dialog->formStub();
      if (!stub)
         return false;
      //
      auto& fi = dovah::form_type_info::lookup(stub->form_type);
      QString label = tr("[%1:%2]%3")
         .arg(cobb::qt::four_cc_to_string(fi.signature))
         .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper())
         .arg(stub->editorID.c_str());
      //
      auto* action = new QAction(this->form_edit_window_menu);
      action->setText(label);
      action->setData(stub->formID);
      QObject::connect(dialog->asDialog(), &QObject::destroyed, action, [action]() {
         action->setData(0);
         action->deleteLater();
      });
      this->form_edit_window_menu->addAction(action);
      //
      return false;
   });
   //
   if (this->form_edit_window_menu->isEmpty()) {
      auto* action = new QAction(this->form_edit_window_menu);
      action->setText(tr("<none>"));
      action->setData(0);
      action->setEnabled(false);
      this->form_edit_window_menu->addAction(action);
   }
}
void MainWindow::updateFormUsesWindowList() {
   this->form_uses_window_menu->clear();
   //
   auto& editor = DovahKitCore::get();
   editor.for_each_form_uses_dialog([this](FormUseInfoDialog* dialog) {
      auto* stub = dialog->formStub();
      if (!stub)
         return false;
      //
      auto& fi = dovah::form_type_info::lookup(stub->form_type);
      QString label = tr("[%1:%2]%3")
         .arg(cobb::qt::four_cc_to_string(fi.signature))
         .arg(QString("%1").arg(stub->formID, 8, 16, QChar('0')).toUpper())
         .arg(stub->editorID.c_str());
      //
      auto* action = new QAction(this->form_uses_window_menu);
      action->setText(label);
      action->setData(stub->formID);
      QObject::connect(dialog, &QObject::destroyed, action, [action]() {
         action->setData(0);
         action->deleteLater();
      });
      this->form_uses_window_menu->addAction(action);
      //
      return false;
   });
   //
   if (this->form_uses_window_menu->isEmpty()) {
      auto* action = new QAction(this->form_uses_window_menu);
      action->setText(tr("<none>"));
      action->setData(0);
      action->setEnabled(false);
      this->form_uses_window_menu->addAction(action);
   }
}

void MainWindow::updateStatusBarWarningsCount(size_t count, size_t count_unread) {
   auto& info = this->_status_bar_widgets.warning_count;

   if (!count) {
      {  // Show the icon as greyscale when no warnings are present.
         auto image = QImage(":/icons/log-window-icons/warning-16.png");
         auto alpha = image.convertToFormat(QImage::Format_Alpha8);
         image.convertTo(QImage::Format_Grayscale16);
         image.convertTo(QImage::Format_ARGB32);
         image.setAlphaChannel(alpha);

         QPixmap pixmap;
         if (pixmap.convertFromImage(image))
            info.icon->setPixmap(pixmap);
      }
      info.label->setText(tr("No warnings logged", "main window status bar: log window warning count"));
      return;
   }

   info.icon->setPixmap(QPixmap(":/icons/log-window-icons/warning-16.png"));
   if (count_unread) {
      info.label->setText(tr("%n warning(s) (%1 unread)", "main window status bar: log window warning count", count).arg(count_unread));
   } else {
      info.label->setText(tr("%n warning(s) logged", "main window status bar: log window warning count", count));
   }

   auto* status_bar = qobject_cast<DKStatusBar*>(this->statusBar());
   if (status_bar) {
      bool flash = count_unread > 0;
      if (flash) {
         auto* subwindow = this->subwindows.log.widget();
         if (subwindow && subwindow->hasFocus())
            flash = false;
      }
      if (flash) {
         status_bar->flash(info.container);
      }
   }
}