#pragma once
#include <array>
#include <string>
#include <QDialog>
#include "ui_FormSubdialogStaticLODMeshes.h" // generated

class FormSubdialogStaticLODMeshes : public QDialog {
   Q_OBJECT;
   public:
      static constexpr const size_t path_count = 4;
      using PathList = std::array<std::string, path_count>;

   protected:
      struct LevelWidgets {
         QCheckBox*        enabled   = nullptr;
         DKGameFilePicker* model     = nullptr; // must not be nullptr
         QPushButton*      propagate = nullptr;
         QString           last_good_path;
      };

   public:
      FormSubdialogStaticLODMeshes(QWidget* parent = nullptr);

      void setPaths(const PathList&);
      PathList getPaths() const;

   public slots:
      void propagateFrom(size_t);
      void updateLevelStates();
      
   protected:
      Ui::FormSubdialogStaticLODMeshes ui;
      std::array<LevelWidgets, path_count> level_widgets;
};