#pragma once
#include <vector>
#include <QDialog>
#include "ui_FormSubdialogScenePackageAction.h" // generated
#include "./FormSubdialogSceneActionBase.h"
#include "./SceneFormVisualEditor_impl/PackageAction.h"

class FormSubdialogScenePackageAction : public FormSubdialogSceneActionBase {
   Q_OBJECT;
   public:
      FormSubdialogScenePackageAction(QWidget* parent = nullptr);

      SceneFormVisualEditor_impl::PackageActionData data;

      // After you write to `data`, call this to push state to the UI.
      virtual void refresh() override;
      
   protected:
      Ui::FormSubdialogScenePackageAction ui;
};