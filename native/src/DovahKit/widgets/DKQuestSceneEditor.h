#pragma once
#include <QWidget>

class SceneDatastore;

class DKQuestSceneEditor : public QWidget {
   Q_OBJECT;
   public:
      DKQuestSceneEditor(QWidget* parent = nullptr);
      
      #pragma region Overrides
         virtual void paintEvent(QPaintEvent* event) override;
      #pragma endregion

      SceneDatastore* datastore() const;
      void setDatastore(SceneDatastore*);

   protected:
      SceneDatastore* datastore = nullptr;
      struct {
         int view_padding  = 30;
         int actor_outdent = 20;
         
         int phase_header_margin         = 5;
         int phase_header_line_count     = 2;
         int phase_conditions_line_count = 4;

         int actor_padding = 10;

         int action_inset = 10;

         int shadow_size = 3;
      } _display_metrics;

      void _update_geometry();
};