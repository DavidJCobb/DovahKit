#pragma once
#include <cstdint>
#include <QWidget>

namespace dovah {
   class form_stub;
}

class DKQuestSceneDialogueActionEditor : public QWidget {
   Q_OBJECT;
   public:
      DKQuestSceneDialogueActionEditor(QWidget* parent = nullptr);

      #pragma region Overrides
         #pragma region Layout
            virtual bool hasHeightForWidth() const override { return true; }
            virtual int heightForWidth(int w) const override;
         #pragma endregion
         #pragma region Events
            virtual void keyPressEvent(QKeyEvent* event) override; // Del key to delete
            virtual void mouseDoubleClickEvent(QMouseEvent* event) override; // open properties
         #pragma endregion

         virtual void paintEvent(QPaintEvent* event) override;
      #pragma endregion

   protected:
      struct {
         uint32_t action_index = 0;
         QString  action_name;
         std::vector<QString> infos_to_display;
      } _cached;
      dovah::form_stub* _topic = nullptr;
};