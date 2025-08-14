#pragma once
#include <QDialog>
#include "./ClimateWeathersModel.h"
#include "ui_FormSubdialogClimateWeather.h" // generated

class FormSubdialogClimateWeather : public QDialog {
   Q_OBJECT;
   public:
      using value_type = ClimateWeathersModelNode;

   public:
      FormSubdialogClimateWeather(QWidget* parent = nullptr);

      void setValue(const value_type&);
      value_type value() const;
      
   protected:
      Ui::FormSubdialogClimateWeather ui;
};


