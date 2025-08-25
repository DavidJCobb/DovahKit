#pragma once
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QWidget>
#include "./DKFormListPane.h"

#if !defined(QT_PLUGIN)
namespace dovah::loaded_forms {
   namespace components {
      struct idle_collection;
   }
   class Form;
}
#endif

class DKIdleCollectionWidget : public QWidget {
   Q_OBJECT;
   public:
      DKIdleCollectionWidget(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
      void importData(const dovah::loaded_forms::components::idle_collection&);
      void exportData(dovah::loaded_forms::components::idle_collection&, dovah::loaded_forms::Form& owner);
      #endif

   protected:
      struct {
         DKFormListPane* idles      = nullptr;
         QComboBox*      order      = nullptr;
         QCheckBox*      do_once    = nullptr;
         QDoubleSpinBox* idle_timer = nullptr;
      } _subwidgets;
};