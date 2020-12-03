#pragma once
#include <cstdint>
#include <QtWidgets/QMainWindow>
#include <QtWinExtras/qwintaskbarbutton.h> // this probably isn't the right way to include this, but Visual Studio and Qt Tools are not being cooperative.
#include "ui_main_window.h"
#include "main_window/cell_view.h"
#include "main_window/object_window.h"
#include "main_window/log_window.h"

class DefaultObjectWindow;
class FileMetadataWindow;
class GameSettingWindow;

class MainWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      MainWindow(QWidget* parent = Q_NULLPTR);
      //
      static MainWindow& get(); // done differently because the usual "static singleton getter" approach apparently causes Qt to crash on exit if applied to the main window
      //
   public slots:
      void setProgressBounds(int, int);
      void setProgressStep(int);
      void setProgressEnableState(bool);
      //
   signals:
      void shown();
      //
   private:
      struct _subwindow_base {
         QWidget*        _widget = nullptr;
         QMdiSubWindow*  _window = nullptr;
         Qt::WindowFlags flags;
         //
         void _open(QMdiArea* parent);
      };
      template<class C> struct _subwindow : public _subwindow_base {
         void open(QMdiArea* parent) {
            if (!this->_widget)
               this->_widget = new C(parent);
            this->_open(parent);
         }
         inline C* widget() const noexcept { return (C*)this->_widget; }
      };
      //
   private:
      Ui::MainWindow ui;
      struct {
         _subwindow<CellViewWindow> cell_view;
         _subwindow<LogWindow>      log;
         _subwindow<ObjectWindow>   object;
      } subwindows;
      QWinTaskbarButton*   taskbar_button        = nullptr;
      DefaultObjectWindow* default_object_window = nullptr;
      GameSettingWindow*   game_setting_window   = nullptr;
      FileMetadataWindow*  metadata_window       = nullptr;
      //
      QMenu* form_uses_window_menu = nullptr;
      QMenu* form_edit_window_menu = nullptr;
      //
      QMdiSubWindow* getSubwindowFor(QWidget*) const noexcept;
      //
   protected:
      virtual void closeEvent(QCloseEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;

      void updateFormEditWindowList();
      void updateFormUsesWindowList();
};
