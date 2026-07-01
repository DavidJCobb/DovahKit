#pragma once
#include <QDialog>
#include <QPointer>
#include "ui_FormSubdialogBodyPartDataBodyPart.h" // generated
#include "dovah/forms/BodyPartData.h"
class SkeletonBonesModel;

class FormSubdialogBodyPartDataBodyPart : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type = dovah::loaded_forms::BodyPartData;
      using loaded_item_type = loaded_form_type::unmanaged_part;

   public:
      FormSubdialogBodyPartDataBodyPart(QWidget* parent = nullptr);

      SkeletonBonesModel* bonesModel() const;
      void setBonesModel(SkeletonBonesModel*);

      [[nodiscard]] loaded_item_type value() const;
      void setValue(const loaded_item_type&);
      
   protected:
      Ui::FormSubdialogBodyPartDataBodyPart ui;
      QPointer<SkeletonBonesModel> bones_model;
};