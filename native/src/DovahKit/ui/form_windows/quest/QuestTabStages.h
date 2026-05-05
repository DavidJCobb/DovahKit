#pragma once
#include <QMenu>
#include <QPointer>
class QCheckBox;
class QListView;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QTableView;
class DKConditionList;
class DKFormPicker;
class DKPapyrusFragmentFunctionPicker;
namespace dovah {
   namespace loaded_forms {
      class Quest;
   }
   class form_stub;
}
class QuestStagesModel;
class QuestStageLogEntriesModel;

class QuestTabStages : public QObject {
   Q_OBJECT;
   private:
      using quest_form_type = dovah::loaded_forms::Quest;

   public:
      QuestTabStages(quest_form_type& quest, QWidget* parent = nullptr);
      ~QuestTabStages();

      void setupUi();

      void load();
      void save();

      struct {
         struct {
            struct {
               QPushButton* create = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            struct {
               QSpinBox*  id                 = nullptr;
               QCheckBox* keep_instance_data = nullptr;
               QCheckBox* shutdown           = nullptr;
               QCheckBox* startup            = nullptr;
            } current;
            QListView* view = nullptr;
         } stages;
         struct {
            struct {
               QPushButton* create = nullptr;
               QPushButton* remove = nullptr;
            } buttons;
            struct {
               QCheckBox*       complete   = nullptr;
               DKConditionList* conditions = nullptr;
               QCheckBox*       fail       = nullptr;
               DKPapyrusFragmentFunctionPicker* fragment = nullptr;
               DKFormPicker*    next_quest = nullptr;
               QPlainTextEdit*  text       = nullptr;
            } current;
            QTableView* view = nullptr;
         } log_entries;
      } ui;
      
   protected:
      quest_form_type& working_quest;
      struct {
         QuestStagesModel*          stages      = nullptr;
         QuestStageLogEntriesModel* log_entries = nullptr;
      } models;

      QModelIndex _selected_stage_qmi();
      void _on_stage_selected();
      void _on_stage_data_edited();

      QModelIndex _selected_log_entry_qmi();
      void _on_log_entry_selected();
      void _on_log_entry_data_edited();
};
