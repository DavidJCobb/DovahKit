#include "./FactionMembersModel.h"
#include "dovah/forms/ActorBase.h"
#include "dovah/form_stub.h"
#include "editor/core.h"

FactionMembersModel::FactionMembersModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
      if (form->form_type != dovah::form_type::actor_base)
         return;
      this->onActorBaseChanged(*form);
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* form, bool just_being_flagged) {
      if (form == this->_faction && this->_faction) {
         this->_faction = nullptr;
         this->clear();
         return;
      }
      if (form->form_type != dovah::form_type::actor_base)
         return;
      this->onActorBaseDeletionImminent(*form);
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      this->_faction = nullptr;
      this->clear();
   });
}

std::optional<int32_t> FactionMembersModel::_actorRank(dovah::form_stub& actor_base) const {
   auto loaded = actor_base.load().ptr_cast<dovah::loaded_forms::ActorBase>();
   if (!loaded)
      return {};

   for (auto& membership : loaded->faction_memberships)
      if (membership.faction == this->_faction)
         return membership.rank;

   //
   // We don't check `loaded->crime_faction` because the crime faction should always 
   // be in the `faction_memberships` list (and should additionally always be a 
   // faction that was flagged as tracking crimes). The file format does not prevent 
   // this rule from being broken, but we're under no obligation to be generous with 
   // bad data here.
   //

   return {};
}
void FactionMembersModel::setFaction(const dovah::form_stub& faction) {
   this->performReset([this, &faction]() {
      this->_faction = &faction;
      for (auto& pair : faction.inbound) {
         auto& info = pair.second;
         auto* user = info.other;
         if (!user || user->form_type != dovah::form_type::actor_base)
            continue;

         auto rank = this->_actorRank(*user);
         if (!rank.has_value())
            continue;
         
         auto* node = new node_type;
         this->_nodes.push_back(node);
         node->actor_base = user;
         node->rank       = rank.value();
         node->cached.editorID = QString::fromStdString(user->editorID);
      }
   });
}

void FactionMembersModel::onActorBaseChanged(dovah::form_stub& actor_base) {
   for (size_t i = 0; i < this->_nodes.size(); ++i) {
      auto& node = this->_nodes[i];
      if (node->actor_base == &actor_base) {
         auto rank = this->_actorRank(actor_base);
         if (!rank.has_value()) {
            //
            // Actor was removed from this faction.
            //
            this->beginRemoveRows({}, i, i);
            this->_nodes.removeAt(i);
            this->endRemoveRows();
            return;
         }

         node->rank = rank.value();
         node->cached.editorID = QString::fromStdString(actor_base.editorID);

         auto qmi = this->index(i, 0, {});
         emit dataChanged(qmi, qmi, { Qt::DisplayRole, Qt::ToolTipRole });
         return;
      }
   }
   //
   // Detect actor bases who were newly given membership in this faction.
   //
   auto rank = this->_actorRank(actor_base);
   if (rank.has_value()) {
      this->beginInsertRows({}, this->_nodes.size(), this->_nodes.size());
      auto* node = new node_type;
      this->_nodes.push_back(node);
      node->actor_base = &actor_base;
      node->rank       = rank.value();
      node->cached.editorID = QString::fromStdString(actor_base.editorID);
      this->endInsertRows();
   }
}
void FactionMembersModel::onActorBaseDeletionImminent(const dovah::form_stub& actor_base) {
   for (size_t i = 0; i < this->_nodes.size(); ++i) {
      auto& node = this->_nodes[i];
      if (node->actor_base == &actor_base) {
         this->deleteItems(i, 1);
         return;
      }
   }
}

QVariant FactionMembersModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole)
      return {};
   switch (section) {
      case 0: return tr("Rank");
      case 1: return tr("Actor Base");
   }
   return {};
}

QVariant FactionMembersModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return {};
   switch (column) {
      case 0:
         return node.rank;
      case 1:
         return node.cached.editorID;
   }
   return {};
}
Qt::ItemFlags FactionMembersModel::flags_of(const node_type& node, size_t column) const {
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}