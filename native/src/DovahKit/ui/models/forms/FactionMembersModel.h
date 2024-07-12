#pragma once
#include <cstdint>
#include <optional>
#include "ui/models/DKGenericListModel.h"

namespace dovah {
   class form_stub;
}

struct FactionMembersModelNode {
   dovah::form_stub* actor_base = nullptr;
   int32_t           rank       = 0;
   struct {
      QString editorID;
   } cached;
};

//
// A model for showing all ActorBases that have membership in a given Faction.
//
class FactionMembersModel : public DKGenericListModel<FactionMembersModel, FactionMembersModelNode> {
   friend DKGenericListModel;
   public:
      static constexpr const size_t column_count = 2;
   public:
      FactionMembersModel(QObject* parent = nullptr);

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      void setFaction(const dovah::form_stub& faction);

   protected:
      std::optional<int32_t> _actorRank(dovah::form_stub& actor_base) const;

      void onActorBaseChanged(dovah::form_stub& actor_base);
      void onActorBaseDeletionImminent(const dovah::form_stub& actor_base);

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;

   protected:
      const dovah::form_stub* _faction = nullptr;
};