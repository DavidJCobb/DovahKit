#pragma once
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/BodyPartData.h"
#include "ui_body_part_data.h" // generated

class BodyPartDataPartsModel;
class SkeletonBonesModel;

class FormDialogBodyPartData :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::BodyPartData, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogBodyPartData(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogBodyPartData ui;
      struct {
         SkeletonBonesModel*     bones = nullptr;
         BodyPartDataPartsModel* parts = nullptr;
      } models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

   public:
      void create_part();
      void edit_part();
      void delete_selected_part();
};
