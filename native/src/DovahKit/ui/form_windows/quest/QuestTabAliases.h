#pragma once
#include <cstdint>
#include <QAction>
#include <QMenu>
#include <QTableView>
#include "dovah/form_stub.h"

namespace dovah {
   namespace loaded_forms {
      class Alias;
      class Quest;
   }
   class form_stub;
}
class QuestAliasesModel;

class QuestTabAliases : public QObject {
   Q_OBJECT;
   private:
      using quest_form_type = dovah::loaded_forms::Quest;
   public:
      QuestTabAliases(quest_form_type& quest, QWidget* parent = nullptr);
      ~QuestTabAliases();

      void setupUi();

      struct {
         struct {
            QMenu menu;
            struct {
               QAction* create_loc = nullptr;
               QAction* create_ref = nullptr;
               QAction* remove     = nullptr;
            } actions;
         } context;
         QTableView* view = nullptr;
      } ui;
      
   protected:
      quest_form_type&   working_quest;
      QuestAliasesModel* model = nullptr;

      void edit_selected_alias();
};
