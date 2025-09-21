#pragma once
#include "./_base.h"
#include "dovah/forms/Furniture.h"
#include "ui_furniture.h"

class FurnitureMarkersModel;
class FurnitureMarkerEntryPointsProxyModel;

class FormDialogFurniture :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Furniture, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogFurniture(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogFurniture ui;
      struct {
         FurnitureMarkersModel*                markers      = nullptr;
         FurnitureMarkerEntryPointsProxyModel* entry_points = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _update_nif_related_flags();
};
