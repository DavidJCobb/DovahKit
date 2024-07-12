#pragma once
#include <cstdint>
#include <QAction>
#include <QButtonGroup>
#include <QMenu>
#include "./_base.h"
#include "dovah/forms/Faction.h"
#include "ui_faction.h" // generated

#include "ui/models/DKGenericListModel.h"

class FactionMembersModel;

class FormDialogFaction :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Faction, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   template<class Self, typename Node> friend class DKGenericListModel;
   public:
      FormDialogFaction(dovah::form_stub& stub, QWidget* parent = nullptr);

   public:
      enum class FactionReaction {
         Ally,
         Friend,
         Neutral,
         Enemy,
      };

      struct ModeledFactionRelationship {
         dovah::form_stub* other    = nullptr;
         int32_t           mod      = 0;
         FactionReaction   reaction = FactionReaction::Neutral;

         QString faction_string;
      };
      using ModeledFactionRank = loaded_form_type::rank;

   protected:
      class RelationshipModel : public DKGenericListModel<RelationshipModel, ModeledFactionRelationship> {
         public:
            static constexpr const size_t column_count = 3;
         public:
            RelationshipModel(QObject* parent = nullptr);

            virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

            QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
            Qt::ItemFlags flags_of(const node_type&, size_t column) const;

            using list_type = decltype(loaded_form_type::relationships);

            void importFrom(const list_type&);
            void commitTo(loaded_form_type& owner, list_type&) const;

            using DKGenericListModel::clear;
            using DKGenericListModel::deleteItems;

            void onFactionChanged(const dovah::form_stub& faction);
            void onFactionDeletionImminent(const dovah::form_stub& faction);

            // Search for multiple entries targeting the same faction and combine them, by 
            // overwriting the earlier entry with data from the later entry and then removing 
            // the later entry.
            void coalesce();

            const node_type* node(const QModelIndex& qmi) const {
               return DKGenericListModel::node(qmi);
            }
            QModelIndex addItem();
            void setFactionData(const QModelIndex&, dovah::form_stub&, int32_t mod, FactionReaction);
      };
      class RanksModel : public DKGenericListModel<RanksModel, ModeledFactionRank> {
         public:
            static constexpr const size_t column_count = 4;
         public:
            using DKGenericListModel::DKGenericListModel;

            virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

            QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
            Qt::ItemFlags flags_of(const node_type&, size_t column) const;

            using list_type = decltype(loaded_form_type::ranks);

            void importFrom(const list_type&);
            void commitTo(loaded_form_type& owner, list_type&) const;

            QModelIndex addItem();
            using DKGenericListModel::clear;
            using DKGenericListModel::deleteItems;

            const node_type* node(const QModelIndex& qmi) const {
               return DKGenericListModel::node(qmi);
            }
            void setRankData(size_t row, const node_type&);
      };
      
   protected:
      Ui::FormDialogFaction ui;
      struct {
         QMenu* ranks         = nullptr;
         QMenu* relationships = nullptr;
      } context_menus;
      struct {
         FactionMembersModel* members       = nullptr;
         RanksModel*          ranks         = nullptr;
         RelationshipModel*   relationships = nullptr;
      } models;
      struct {
         QButtonGroup* faction_alliance_status = nullptr;
      } subwidgets;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;
};
