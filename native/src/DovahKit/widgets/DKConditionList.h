#pragma once
#include <vector>
#include <QMenu>
#include <QWidget>
#if !defined(QT_PLUGIN)
   #include "ui/types/conditions/condition.h"
#endif

class QPushButton;
class QTableView;
#if !defined(QT_PLUGIN)
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
      #if !defined(QT_PLUGIN)
      using BackendCondition     = dovah::loaded_forms::components::condition;
      using BackendConditionList = dovah::loaded_forms::components::condition_list;
      #endif

   public:
      DKConditionList(QWidget* parent = nullptr);

      #if !defined(QT_PLUGIN)
         void importFrom(dovah::loaded_forms::Form& owner, const BackendConditionList& target);
         void importFrom(dovah::loaded_forms::Form& owner, const std::vector<ui::types::conditions::condition>& target);
         void exportTo(dovah::loaded_forms::Form& owner, BackendConditionList& target);
         void exportTo(dovah::loaded_forms::Form& owner, std::vector<ui::types::conditions::condition>& target);
         void clear();

         void overrideOwningForm(dovah::loaded_forms::Form&);

         // These exist to handle edge-cases with TopicInfo conditions.
         void importBifurcatedList(dovah::loaded_forms::Form& owner, const BackendConditionList& locked, const BackendConditionList& normal);
         void exportBifurcatedList(dovah::loaded_forms::Form& owner, BackendConditionList& locked, BackendConditionList& normal);
      #endif

      size_t conditionCount() const;

      // Del key on listview
      virtual bool eventFilter(QObject* object, QEvent* event) override;

   signals:
      void changeAttempted();
      void changed();

   public slots:
      void openCreateConditionModal();
      void openEditConditionModal();

   protected:
      struct {
         QMenu menu;
         struct {
            QAction* edit      = nullptr;
            QAction* move_up   = nullptr;
            QAction* move_down = nullptr;
            QAction* remove    = nullptr;
         } actions;
      } _context;
      struct {
         QPushButton* add_item    = nullptr;
         QPushButton* move_up     = nullptr;
         QPushButton* move_down   = nullptr;
         QPushButton* remove_item = nullptr;
         QTableView*  view        = nullptr;
      } _subwidgets;
      #if !defined(QT_PLUGIN)
         DKConditionListModel* _model = nullptr;
         dovah::form_stub* _owning_stub = nullptr;
      #endif

      bool _has_selection() const;
      void _update_button_enable_states();

      #if !defined(QT_PLUGIN)
         void _move_selection_up();
         void _move_selection_down();
         void _delete_selection();

         void _copy_selected();
         void _paste();
      #endif
};