#pragma once
#include "ui/models/DKGenericListModel.h"

#include "dovah/forms/structs/movement_type_speeds.h"

namespace dovah {
   namespace loaded_forms {
      class AssociationType;
      class Relationship;
   }
   class form_stub;
}

struct RaceMovementDataOverridesModelNode {
   dovah::form_stub* movement_type = nullptr;
   dovah::loaded_forms::structs::movement_type_speeds speeds;
   //
   struct {
      QString editor_id;
   } cached;

   void recache_editor_id();
};

class RaceMovementDataOverridesModel : public DKGenericListModel<RaceMovementDataOverridesModel, RaceMovementDataOverridesModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            EditorID,
            FormID,

            __COUNT
         };
      };
      static constexpr const size_t column_count = Column::__COUNT; // override

   protected:
      struct {
         dovah::sex sex = dovah::sex::female;
         const dovah::form_stub* stub = nullptr;
      } _focus_actor;

   public:
      ActorBaseRelationshipsModel(QObject* parent);

      using DKGenericListModel::clear;

      #pragma region Overrides
         QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const;
         Qt::ItemFlags flags_of(const node_type&, size_t column) const;

         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      // Also clears the model.
      void setFocusActor(const dovah::form_stub*, dovah::sex);

      // Requires that a focus actor have been set.
      void addRelationship(dovah::form_stub& relationship);

      dovah::form_stub* relationshipAt(size_t row) const {
         if (row >= this->_nodes.size())
            return nullptr;
         return this->_nodes[row]->relationship;
      }
};
