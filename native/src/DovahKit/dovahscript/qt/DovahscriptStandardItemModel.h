#pragma once
#include "../../editor/scripting/ui/util/lua_item_model.h" // TODO: Move this to a different file

class DovahscriptStandardItemModel : public ObservableStandardItemModel {
   protected:
      QList<QWidget*> _associated_widgets;

   protected slots:
      void associatedWidgetDestroyed(QWidget*);

   public:
      QList<QWidget*> associatedWidgets();
      void associateWithWidget(QWidget*);
      void dissociateFromWidget(QWidget*);

   signals:
      void dissociatedFromAll();
};
