#pragma once
#include <QPushButton>
#include <QTableView>

class DKPapyrusScriptObjectListModel;
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/core.h"
   #include "dovah/form_stub.h"
   #include "dovah/forms/components/papyrus.h"
   #include "./widget-models/DKPapyrusScriptObjectListModel.h"
#endif

namespace dovah {
   class form_stub;
}

class DKPapyrusScriptObjectWidget : public QWidget {
   Q_OBJECT;
   public:
      using data_type = dovah::loaded_forms::components::papyrus_attachment_data;

   public:
      DKPapyrusScriptObjectWidget(QWidget* parent);

   public slots:
      #if !defined(QT_DESIGNER_LIB)
         void setTarget(dovah::form_stub*);
      #endif

   signals:

   protected:
      struct {
         struct {
            QWidget* wrapper = nullptr;
            //
            QPushButton* add           = nullptr;
            QPushButton* toggle_delete = nullptr; // for inherited scripts, switches between Remove and Undelete
            QPushButton* properties    = nullptr;
         } buttons;
         QTableView* view = nullptr;
      } subwidgets;
      #if !defined(QT_DESIGNER_LIB)
      struct {
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> parent;
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> target;
      } loaded_forms;
      #endif
      //
      DKPapyrusScriptObjectListModel* model = nullptr;

      #if !defined(QT_DESIGNER_LIB)
      void _editSelected();
      void _updateButtons();
      #endif
};