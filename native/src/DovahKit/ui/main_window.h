#pragma once
#include <cstdint>
#include <QLabel>
#include <QMainWindow>
#include <QMdiSubWindow>
#include "ui_main_window.h"

class DefaultObjectWindow;
class FileMetadataWindow;
class GameSettingWindow;

class CellViewWindow;
class LogWindow;
class ObjectWindow;
class RenderWindow;

class QWinTaskbarButton;

class MainWindow : public QMainWindow {
   Q_OBJECT
   //
   public:
      MainWindow(QWidget* parent = Q_NULLPTR);
      ~MainWindow();
      //
      static MainWindow& get();
      
   public slots:
      void setProgressBounds(int, int);
      void setProgressStep(int);
      void setProgressEnableState(bool);
      
   signals:
      void shown();
      
   private:
      struct _subwindow_base {
         QWidget*        _widget = nullptr;
         QMdiSubWindow*  _window = nullptr;
         Qt::WindowFlags flags;
         QMdiSubWindow::SubWindowOptions options;
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
      
   private:
      Ui::MainWindow ui;
      struct {
         struct {
            QWidget* container = nullptr;
            QLabel*  icon      = nullptr;
            QLabel*  label     = nullptr;
         } warning_count;
      } _status_bar_widgets;
      struct {
         _subwindow<CellViewWindow> cell_view;
         _subwindow<LogWindow>      log;
         _subwindow<ObjectWindow>   object;
         _subwindow<RenderWindow>   render;
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
      
   protected:
      virtual void closeEvent(QCloseEvent* event) override;
      virtual void showEvent(QShowEvent* event) override;

      void updateFormEditWindowList();
      void updateFormUsesWindowList();

      void updateStatusBarWarningsCount(size_t count, size_t count_unread);
};
