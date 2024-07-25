#include "./ActorBaseRelationshipsModel.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

#include "dovah/forms/AssociationType.h"
#include "dovah/forms/Relationship.h"

namespace {
   bool _relationship_may_include(const dovah::form_stub& relationship, const dovah::form_stub& actor) {
      for (auto& pair : relationship.outbound) {
         auto& info = pair.second;
         if (info.other == &actor)
            return true;
      }
      return false;
   }
}

void ActorBaseRelationshipsModelNode::recache_association_typename(const dovah::form_stub* actor, dovah::sex actor_sex, bool is_referrer) {
   if (!this->association) {
      this->cached.associationTypeName = "";
      return;
   }
   auto loaded = this->association->load().ptr_cast<dovah::loaded_forms::AssociationType>();
   if (loaded) {
      if (is_referrer) {
         this->cached.associationTypeName = QString::fromStdString(actor_sex == dovah::sex::female ? loaded->referrer.fem : loaded->referrer.masc);
      } else {
         this->cached.associationTypeName = QString::fromStdString(actor_sex == dovah::sex::female ? loaded->referent.fem : loaded->referent.masc);
      }
   }
}
void ActorBaseRelationshipsModelNode::recache_other_actor_editor_id() {
   if (!this->other_actor) {
      this->cached.otherActorEditorID = "";
      return;
   }
   this->cached.otherActorEditorID = QString::fromStdString(this->other_actor->editorID);
}

//

ActorBaseRelationshipsModel::ActorBaseRelationshipsModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &ActorBaseRelationshipsModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      size_t size = this->_nodes.size();
      for(size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (stub == node->relationship) {
            auto loaded = stub->load().ptr_cast<dovah::loaded_forms::Relationship>();
            if (!loaded)
               continue;
            bool is_referrer = loaded->referrer == this->_focus_actor.stub;
            if (!is_referrer && loaded->referent != this->_focus_actor.stub) {
               //
               // Relationship no longer refers to the focus actor.
               //
               this->beginRemoveRows({}, i, i);
               --i;
               --size;
               delete node;
               this->_nodes.erase(this->_nodes.begin() + i);
               this->endRemoveRows();
               continue;
            }
            {
               auto* prior_other = node->other_actor;
               if (is_referrer) {
                  node->other_actor = loaded->referent.get_form_stub();
               } else {
                  node->other_actor = loaded->referrer.get_form_stub();
               }
               node->other_actor_is_referent = is_referrer;
               node->recache_other_actor_editor_id();
               if (prior_other != node->other_actor) {
                  auto qmi = this->index(i, Column::OtherActor, {});
                  emit dataChanged(qmi, qmi, {});
               }
            }
            if (node->relationship_level != loaded->rank) {
               node->relationship_level = loaded->rank;

               auto qmi = this->index(i, Column::RelationshipRank, {});
               emit dataChanged(qmi, qmi, {});
            }
         } else if (stub == node->association) {
            node->recache_association_typename(this->_focus_actor.stub, this->_focus_actor.sex, node->other_actor_is_referent);

            auto qmi = this->index(i, Column::AssociationTypeName, {});
            emit dataChanged(qmi, qmi, {});
         } else if (stub == node->other_actor) {
            node->recache_other_actor_editor_id();

            auto qmi = this->index(i, Column::OtherActor, {});
            emit dataChanged(qmi, qmi, {});
         }
      }
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool just_being_flagged) {
      if (stub == this->_focus_actor.stub) {
         this->_focus_actor.stub = nullptr;
         this->clear();
         return;
      }
      size_t size = this->_nodes.size();
      for (size_t i = 0; i < size; ++i) {
         auto* node = this->_nodes[i];
         if (node->relationship == stub) {
            this->beginRemoveRows({}, i, i);
            this->_nodes.erase(this->_nodes.begin() + i);
            delete node;
            --i;
            --size;
            this->endRemoveRows();
            continue;
         }
         if (node->association == stub) {
            node->association = nullptr;
            node->cached.associationTypeName = "";

            auto qmi = this->index(i, Column::AssociationTypeName, {});
            emit dataChanged(qmi, qmi);
         }
         if (node->other_actor == stub) {
            node->other_actor = nullptr;
            node->cached.otherActorEditorID = "";

            auto qmi = this->index(i, Column::OtherActor, {});
            emit dataChanged(qmi, qmi);
         }
      }
   });
}

QVariant ActorBaseRelationshipsModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   switch (role) {
      case Qt::TextAlignmentRole:
         switch (column) {
            case Column::RelationshipRank:
            case Column::AssociationTypeName:
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
         }
         return {};

      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         switch (column) {
            case Column::OtherActor:
               return node.cached.otherActorEditorID;
            case Column::RelationshipRank:
               switch (node.relationship_level) {
                  using enum node_type::RelationshipRank;
                  case lover:
                     return tr("Lover");
                  case ally:
                     return tr("Ally");
                  case confidant:
                     return tr("Confidant");
                  case friend_:
                     return tr("Friend");
                  case acquaintance:
                     return tr("Acquaintance");
                  case rival:
                     return tr("Rival");
                  case foe:
                     return tr("Foe");
                  case enemy:
                     return tr("Enemy");
                  case archnemesis:
                     return tr("Archnemesis");
               }
               break;
            case Column::AssociationTypeName:
               return node.cached.associationTypeName;
         }
         return {};
   }
   return {};
}
Qt::ItemFlags ActorBaseRelationshipsModel::flags_of(const node_type&, size_t column) const {
   auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemNeverHasChildren;
   return flags;
}

/*virtual*/ QVariant ActorBaseRelationshipsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (role != Qt::DisplayRole)
      return {};
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   switch (section) {
      using enum Column::enumeration;
      case OtherActor:
         return tr("Relationship NPC");
      case RelationshipRank:
         return tr("Rank");
      case AssociationTypeName:
         return tr("Type");
   }
   return {};
}

void ActorBaseRelationshipsModel::addRelationship(dovah::form_stub& relationship) {
   if (!this->_focus_actor.stub)
      return;
   if (!_relationship_may_include(relationship, *this->_focus_actor.stub))
      return;

   auto loaded = relationship.load().ptr_cast<dovah::loaded_forms::Relationship>();
   if (!loaded)
      return;
   bool is_referrer = loaded->referrer == this->_focus_actor.stub;
   if (!is_referrer && loaded->referent != this->_focus_actor.stub)
      return;

   this->beginInsertRows({}, this->_nodes.size(), this->_nodes.size());
   auto* node = new node_type;
   this->_nodes.push_back(node);

   node->relationship = &relationship;
   node->association  = loaded->association_type.get_form_stub();
   node->relationship_level = loaded->rank;
   node->recache_association_typename(this->_focus_actor.stub, this->_focus_actor.sex, is_referrer);

   node->other_actor_is_referent = is_referrer;
   if (is_referrer) {
      node->other_actor = loaded->referent.get_form_stub();
   } else {
      node->other_actor = loaded->referrer.get_form_stub();
   }
   node->recache_other_actor_editor_id();

   this->endInsertRows();
}
void ActorBaseRelationshipsModel::setFocusActor(const dovah::form_stub* stub, dovah::sex sex) {
   auto prior = this->_focus_actor;
   this->_focus_actor = {
      .sex  = sex,
      .stub = stub,
   };
   if (prior.stub != stub) {
      this->clear();
      return;
   }
   if (prior.sex != sex) {
      for (size_t i = 0; i < this->_nodes.size(); ++i) {
         auto* node = this->_nodes[i];
         node->recache_association_typename(this->_focus_actor.stub, this->_focus_actor.sex, node->other_actor_is_referent);

         auto qmi = this->index(i, Column::AssociationTypeName, {});
         emit dataChanged(qmi, qmi, {});
      }
   }
}