#pragma once
#include "../../editor/scripting/ui/util/lua_item_model.h" // TODO: Move this to a different file

class DovahscriptStandardItemModel : public ObservableStandardItemModel {
   Q_OBJECT;
   protected:
      QList<QWidget*> _associated_widgets;

   protected slots:
      void associatedWidgetDestroyed(QWidget*);

   public:
      using ObservableStandardItemModel::ObservableStandardItemModel;

      QList<QWidget*> associatedWidgets();
      void associateWithWidget(QWidget*);
      void dissociateFromWidget(QWidget*);

   signals:
      void dissociatedFromAll();
};
