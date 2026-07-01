#pragma once
#include <QDialog>
#include "ui_FormSubdialogSceneProperties.h" // generated
namespace dovah::loaded_forms {
   namespace components::papyrus {
      class scene_fragment_data;
   }
   class Scene;
}

class FormSubdialogSceneProperties : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type           = dovah::loaded_forms::Scene;
      using papyrus_fragment_data_type = dovah::loaded_forms::components::papyrus::scene_fragment_data;

   public:
      FormSubdialogSceneProperties(QWidget* parent = nullptr);

      void importData(loaded_form_type&);
      void exportData(loaded_form_type&) const;
      
   protected:
      Ui::FormSubdialogSceneProperties ui;
};