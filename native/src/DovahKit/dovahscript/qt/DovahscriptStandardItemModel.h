#pragma once
#include "../../ui/generic/ObservableStandardItemModel.h"

class DovahscriptStandardItemModel : public ObservableStandardItemModel {
   Q_OBJECT;
   protected:
      QList<QWidget*> _associated_widgets;

   protected slots:
      void associatedWidgetDestroyed(QObject*);

   public:
      using ObservableStandardItemModel::ObservableStandardItemModel;

      QList<QWidget*> associatedWidgets();
      void associateWithWidget(QWidget*);
      void dissociateFromWidget(QWidget*);

   signals:
      void dissociatedFromAll();
};
