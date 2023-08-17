#pragma once
#include <vector>
#include <QDialog>
#include "ui_options_window.h"

namespace cobb::ini {
   class setting;
}

class OptionsWindow : public QDialog {
   Q_OBJECT;
   public:
      OptionsWindow(QWidget* parent = nullptr);
      ~OptionsWindow();

      static OptionsWindow* open(QWidget* parent = nullptr);

   private:
      static OptionsWindow* instance;

   protected:
      struct ini_widget_basic_mapping {
         cobb::ini::setting* setting = nullptr;
         QWidget*            widget  = nullptr;
      };
      struct ini_widget_radio_bool_mapping {
         cobb::ini::setting* setting = nullptr;
         QRadioButton* widget_true  = nullptr;
         QRadioButton* widget_false = nullptr;
      };

   public slots:
      void revertChanges();
      void save();
      
   private:
      Ui::OptionsWindow ui;
      struct {
         std::vector<ini_widget_basic_mapping> basic;
         std::vector<ini_widget_radio_bool_mapping> radio_bool;
      } _mappings;
};