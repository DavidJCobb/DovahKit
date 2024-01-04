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

namespace dovah::loaded_forms {
   namespace components::papyrus {
      class attachment_data;
   }
   class Form;
}
namespace dovah {
   class form_stub;
}

class DKPapyrusScriptObjectWidget : public QWidget {
   Q_OBJECT;
   public:
      using vmad_type         = dovah::loaded_forms::components::papyrus::attachment_data;
      using working_copy_type = dovah::loaded_forms::Form;

   public:
      DKPapyrusScriptObjectWidget(QWidget* parent);

   public slots:
      #if !defined(QT_DESIGNER_LIB)
         void setFormWorkingCopy(working_copy_type* working_copy);
         void setQuestWorkingCopyAndAliasVMAD(working_copy_type* quest_working_copy, vmad_type& target);
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
         working_copy_type* form = nullptr;
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> base_form; // if `form` is a ref

         vmad_type* parent = nullptr;
         vmad_type* target = nullptr;
      } vmad;
      #endif
      //
      DKPapyrusScriptObjectListModel* model = nullptr;

      #if !defined(QT_DESIGNER_LIB)
      void _editSelected();
      void _updateButtons();
      #endif
};