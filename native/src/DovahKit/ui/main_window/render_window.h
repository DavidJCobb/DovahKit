#pragma once
#include <QToolBar>
#include <QWidget>

// use QWidget because we're gonna stuff it into a QMdiSubWindow or whatever
class RenderWindow : public QWidget {
   Q_OBJECT;
   public:
      RenderWindow(QWidget* parent = nullptr);
      
   private:
      //Ui::RenderWindow ui;
      QToolBar* toolbar = nullptr;
};