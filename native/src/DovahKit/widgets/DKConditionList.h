#pragma once
#include <vector>
#include <QWidget>
#if !defined(QT_DESIGNER_LIB)
   #include "ui/types/conditions/condition.h"
#endif

class QPushButton;
class QTableView;
#if !defined(QT_DESIGNER_LIB)
   namespace dovah {
      namespace loaded_forms {
         namespace components {
            class condition;
            class condition_list;
         }
         class Form;
      }
      class form_stub;
   }
   class DKConditionListModel;
#endif

class DKConditionList : public QWidget {
   Q_OBJECT;
   public:
      #if !defined(QT_DESIGNER_LIB)
      using BackendCondition     = dovah::loaded_forms::components::condition;
      using BackendConditionList = dovah::loaded_forms::components::condition_list;
      #endif

   public:
      DKConditionList(QWidget* parent = nullptr);

      #if !defined(QT_DESIGNER_LIB)
         void importFrom(dovah::loaded_forms::Form& owner, const BackendConditionList& target);
         void importFrom(dovah::loaded_forms::Form& owner, const std::vector<ui::types::conditions::condition>& target);
         void exportTo(dovah::loaded_forms::Form& owner, BackendConditionList& target);
         void exportTo(dovah::loaded_forms::Form& owner, std::vector<ui::types::conditions::condition>& target);
         void clear();

         // These exist to handle edge-cases with TopicInfo conditions.
         void importBifurcatedList(dovah::loaded_forms::Form& owner, const BackendConditionList& locked, const BackendConditionList& normal);
         void exportBifurcatedList(dovah::loaded_forms::Form& owner, BackendConditionList& locked, BackendConditionList& normal);
      #endif

   signals:
      void changeAttempted();
      void changed();

   public slots:
      void openCreateConditionModal();
      void openEditConditionModal();

   protected:
      struct {
         QPushButton* add_item  = nullptr;
         QPushButton* move_up   = nullptr;
         QPushButton* move_down = nullptr;
         QTableView*  view      = nullptr;
      } _subwidgets;
      #if !defined(QT_DESIGNER_LIB)
         DKConditionListModel* _model = nullptr;
         dovah::form_stub* _owning_stub = nullptr;
      #endif
};