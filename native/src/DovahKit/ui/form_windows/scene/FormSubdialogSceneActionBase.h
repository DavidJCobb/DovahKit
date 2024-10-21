#pragma once
#include <cstdint>
#include <utility> // std::pair
#include <vector>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include "./SceneFormVisualEditor_impl/Action.h"

class FormSubdialogSceneActionBase : public QDialog {
   Q_OBJECT;
   public:
      FormSubdialogSceneActionBase(QWidget* parent = nullptr);

      SceneFormVisualEditor_impl::BaseActionData base_data;
      struct {
         std::vector<std::pair<uint32_t, QString>> actors;
         std::vector<QString> phases;
      } scene_data;

      virtual void refresh() = 0;

   protected:
      // After you write to `data`, call this to push state to the UI.
      void _refresh_base(
         QLineEdit* name,
         QComboBox* actor,
         QComboBox* phase_start,
         QComboBox* phase_end
      );
      bool _set_up_signals = false;
};
