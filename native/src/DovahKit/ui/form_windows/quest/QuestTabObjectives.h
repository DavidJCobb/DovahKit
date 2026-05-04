#pragma once
#include <QMenu>
#include <QPointer>
class DKConditionList;
class QAction;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QTableView;
namespace dovah {
   namespace loaded_forms {
      class Quest;
   }
   class form_stub;
}
class QuestAliasesModel;
class QuestObjectivesModel;
class QuestObjectiveTargetsModel;

class QuestTabObjectives : public QObject {
   Q_OBJECT;
   private:
      using quest_form_type = dovah::loaded_forms::Quest;

   protected:
      struct context_menu_ui {
         QMenu menu;
         struct {
            QAction* create = nullptr;
            QAction* remove = nullptr;
         } actions;
      };

   public:
      QuestTabObjectives(quest_form_type& quest, QWidget* parent = nullptr);
      ~QuestTabObjectives();

      void setupUi();
      void setAliasesModel(const QuestAliasesModel*);

      void load();
      void save();

      struct {
         struct {
            struct {
               QCheckBox* flag_or = nullptr;
               QSpinBox*  index   = nullptr;
               QLineEdit* text    = nullptr;
            } current;
            QTableView* view = nullptr;
         } objectives;
         struct {
            struct {
               QComboBox*       alias        = nullptr;
               DKConditionList* conditions   = nullptr;
               QCheckBox*       ignore_locks = nullptr;
            } current;
            QTableView* view = nullptr;
         } targets;
      } ui;
      struct {
         context_menu_ui objectives;
         context_menu_ui targets;
      } context;
      
   protected:
      QPointer<const QuestAliasesModel> aliases_model;
      quest_form_type& working_quest;
      struct {
         QuestObjectivesModel*       objectives = nullptr;
         QuestObjectiveTargetsModel* targets    = nullptr;
      } models;

      void _populate_alias_picker();
      void _re_sort_alias_picker();

      QModelIndex _selected_objective_qmi();
      void _on_objective_selected();
      void _on_objective_data_edited();
      QModelIndex _selected_target_qmi();
      void _on_target_selected();
      void _on_target_data_edited();
};
