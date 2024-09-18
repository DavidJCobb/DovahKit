#pragma once
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         class attack_data;
      }
      class Form;
   }
   class form_stub;
}

struct DKDialogueFormPickerModelNode {
   dovah::form_stub* quest   = nullptr;
   dovah::form_stub* branch  = nullptr;
   dovah::form_stub* scene   = nullptr;
   dovah::form_stub* topic   = nullptr;
   size_t            subtype = (size_t)-1; // index
   struct {
      QString quest_editor_id;
      QString branch_editor_id;
      QString scene_editor_id;
      QString topic_editor_id;
   } cached;

   constexpr dovah::form_stub* innermost_form() const {
      if (auto* stub = this->topic)
         return stub;
      if (auto* stub = this->branch)
         return stub;
      if (auto* stub = this->scene)
         return stub;
      if (auto* stub = this->quest)
         return stub;
      return nullptr;
   }

   void recache();
   void recache_topic();

   void update_quest_if_transplanted();
   void update_branch_if_transplanted();
};

class DKDialogueFormPickerModel : public DKGenericListModel<DKDialogueFormPickerModel, DKDialogueFormPickerModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            Quest,
            BranchOrScene,
            Topic,
            FormID, // of most deeply nested form
            Subtype,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

      static constexpr const Qt::ItemDataRole FormStubRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 0);

   public:
      DKDialogueFormPickerModel(QObject* parent);

   protected:
      dovah::form_type  desired_type = dovah::form_type::none;
      dovah::form_stub* quest        = nullptr;
      dovah::form_stub* form_to_link = nullptr; // INFO, when picking a DIAL to link the INFO to
      dovah::form_stub* form_to_move = nullptr; // DLBR, DIAL, or INFO, when picking a form to transplant elsewhere
      struct {
         QString quest_editor_id;
      } cached;

   public:
      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void clear();

      void setDesiredFormType(dovah::form_type);
      void setQuest(dovah::form_stub*);
      void setFormToLink(dovah::form_stub*);
      void setFormToMove(dovah::form_stub*);

      void refill();

   protected:
      void _on_form_modified(dovah::form_stub*);
      void _on_form_renumbered(dovah::form_stub*);
      void _on_form_deleted(dovah::form_stub*);
};
