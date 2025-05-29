#pragma once
#include "ui_DKMagicEffectListItemDialog.h" // generated
#include <cstdint>
#include <QDialog>
#include "../widget-models/DKMagicEffectListModel.h"

namespace dovah::loaded_forms {
   class Form;
}
class DKMagicEffectListWidget;

class DKMagicEffectListItemDialog : public QDialog {
   Q_OBJECT;
   public:
      DKMagicEffectListItemDialog(DKMagicEffectListWidget&, dovah::loaded_forms::Form&);

      [[nodiscard]] DKMagicEffectListModel::Item data() const;
      void setData(const DKMagicEffectListModel::Item&);

   protected:
      Ui::DKMagicEffectListItemDialog ui;
      DKMagicEffectListWidget&   _owner;
      dovah::loaded_forms::Form& _form;
      struct {
         uint32_t spell_total_cost = 0; // not including the effect we're editing
         uint32_t spell_level      = 0; // not including the effect we're editing
      } _state;
};