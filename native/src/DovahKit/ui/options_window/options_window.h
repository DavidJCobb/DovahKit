#pragma once
#include <vector>
#include <QDialog>
#include "ui_options_window.h" // generated
#include "dovah/data/game.h"
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

      struct game_path_mapping {
         public:
            game_path_mapping(dovah::game g) : game(g) {}

         public:
            const dovah::game game;
            cobb::ini::setting* setting = nullptr;
            struct {
               QPushButton*  browse = nullptr;
               struct {
                  QLineEdit* automatic = nullptr;
                  QLineEdit* manual    = nullptr;
               } paths;
               QRadioButton* use_automatic = nullptr;
               QRadioButton* use_manual    = nullptr;
            } widgets;

         public:
            void init(QWidget* parent, cobb::ini::setting&, decltype(widgets)&&);
            void load();
            void save();
      };

   public slots:
      void revertChanges();
      void save();
      
   private:
      Ui::OptionsWindow ui;
      struct {
         std::vector<ini_widget_basic_mapping> basic;
         std::vector<ini_widget_radio_bool_mapping> radio_bool;
         struct {
            game_path_mapping classic = game_path_mapping(dovah::game::skyrim_classic);
            game_path_mapping special = game_path_mapping(dovah::game::skyrim_special);
         } game_paths;
      } _mappings;
};