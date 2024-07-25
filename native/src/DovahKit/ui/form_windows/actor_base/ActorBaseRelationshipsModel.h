#pragma once
#include <cstdint>
#include "ui/models/DKGenericListModel.h"

#include "dovah/data/sex.h"
#include "dovah/forms/Relationship.h"

namespace dovah {
   namespace loaded_forms {
      class AssociationType;
      class Relationship;
   }
   class form_stub;
}

struct ActorBaseRelationshipsModelNode {
   using RelationshipRank = decltype(dovah::loaded_forms::Relationship::rank);

   dovah::form_stub* relationship = nullptr; // RELA
   dovah::form_stub* association  = nullptr; // ASTP

   dovah::form_stub* other_actor = nullptr;
   bool other_actor_is_referent = false;
   RelationshipRank  relationship_level = RelationshipRank::acquaintance;

   struct {
      QString otherActorEditorID;
      QString associationTypeName; // the name in the ASTP that refers to this actor
   } cached;

   void recache_association_typename(const dovah::form_stub* focus_actor, dovah::sex focus_sex, bool is_referrer);
   void recache_other_actor_editor_id();
};

class ActorBaseRelationshipsModel : public DKGenericListModel<ActorBaseRelationshipsModel, ActorBaseRelationshipsModelNode> {
   public:
      struct Column {
         Column() = delete;
         enum enumeration : size_t {
            OtherActor,
            RelationshipRank,
            AssociationTypeName,

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
