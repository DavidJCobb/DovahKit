#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_game_setting_window.h"

class GameSettingWindow : public QDialog {
   Q_OBJECT
   //
   public:
      GameSettingWindow(QWidget* parent);
      //
   private:
      Ui::GameSettingWindow ui;
};
