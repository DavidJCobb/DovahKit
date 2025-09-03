#pragma once
#include <cstdint>
#include <optional>
#include <QDialog>
#include "ui_FormSubdialogActorValueInfoPerkTree.h" // generated
namespace dovah {
   class form_stub;
}
class SkillTreeVisualEditor;

class FormSubdialogActorValueInfoPerkTree : public QDialog {
   Q_OBJECT;
   public:
      FormSubdialogActorValueInfoPerkTree(QWidget* parent = nullptr);

      void setOwningActorValue(dovah::form_stub*);
      
   protected:
      Ui::FormSubdialogActorValueInfoPerkTree ui;
      struct {
         SkillTreeVisualEditor* editor = nullptr;
      } _widgets;

      void _push_perk_node(std::optional<uint32_t> node_id = {});
      void _pull_perk_node(std::optional<uint32_t> node_id = {});
};