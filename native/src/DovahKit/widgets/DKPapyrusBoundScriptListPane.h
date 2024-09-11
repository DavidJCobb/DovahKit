#pragma once
#include <QGroupBox>
#include <QPushButton>
#include <QTableView>

class DKBoundScriptListModel;
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/core.h"
   #include "dovah/form_stub.h"
   #include "dovah/forms/components/papyrus.h"
   #include "./widget-models/DKBoundScriptListModel.h"
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

class DKPapyrusBoundScriptListPane : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool usesGroupbox READ usesGroupbox WRITE setUsesGroupbox DESIGNABLE true);
   public:
      using vmad_type         = dovah::loaded_forms::components::papyrus::attachment_data;
      using working_copy_type = dovah::loaded_forms::Form;

   public:
      DKPapyrusBoundScriptListPane(QWidget* parent);

   public slots:
      #if !defined(QT_DESIGNER_LIB)
         void setFormWorkingCopy(working_copy_type* working_copy);
         void setQuestWorkingCopyAndAliasVMAD(working_copy_type* quest_working_copy, vmad_type& target);

         void commit();
      #endif

      constexpr bool usesGroupbox() const noexcept { return this->state.use_groupbox; }
      void setUsesGroupbox(bool);

   signals:

   protected:
      struct {
         bool use_groupbox = true;
      } state;
      struct {
         struct {
            QWidget* wrapper = nullptr;
            //
            QPushButton* add           = nullptr;
            QPushButton* toggle_delete = nullptr; // for inherited scripts, switches between Remove and Undelete
            QPushButton* properties    = nullptr;
         } buttons;
         QTableView* view = nullptr;

         QGroupBox* groupbox = nullptr;
         QWidget*   wrapper  = nullptr;
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
      DKBoundScriptListModel* model = nullptr;

      void _updateGroupbox();
      #if !defined(QT_DESIGNER_LIB)
      void _editSelected();
      void _updateButtons();
      #endif
};