#pragma once
#include <QDialog>
#include "ui_FormSubdialogLensFlareSprite.h" // generated
#include "dovah/forms/LensFlare.h"

class FormSubdialogLensFlareSprite : public QDialog {
   Q_OBJECT;
   public:
      using value_type = dovah::loaded_forms::LensFlare::sprite;

   public:
      FormSubdialogLensFlareSprite(QWidget* parent = nullptr);

      value_type value() const;
      void setValue(const value_type&);
      
   protected:
      Ui::FormSubdialogLensFlareSprite ui;
};