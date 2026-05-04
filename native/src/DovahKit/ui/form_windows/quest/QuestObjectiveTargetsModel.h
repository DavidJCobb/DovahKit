#pragma once
#include <QIdentityProxyModel>
#include "./QuestObjectivesModel.h"

class QuestObjectiveTargetsModel : public QIdentityProxyModel {
   public:
      using underlying_model_type = QuestObjectivesModel;

      using Column = underlying_model_type::TargetColumn;
      static constexpr const size_t ColumnCount = underlying_model_type::TargetColumnCount;

      static constexpr const Qt::ItemDataRole AliasIDRole     = underlying_model_type::TargetAliasIDRole;
      static constexpr const Qt::ItemDataRole IgnoreLocksRole = underlying_model_type::TargetIgnoreLocksRole;

      using condition_list = underlying_model_type::condition_list;

   public:
      using QIdentityProxyModel::QIdentityProxyModel;

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex createTarget(const QModelIndex& objective);
      void deleteTarget(const QModelIndex& target);

      condition_list targetConditions(const QModelIndex&) const;
      void setTargetConditions(const QModelIndex&, condition_list&&);
};