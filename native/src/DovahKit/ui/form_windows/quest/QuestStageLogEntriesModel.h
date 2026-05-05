#pragma once
#include <QIdentityProxyModel>
#include "./QuestStagesModel.h"

class QuestStageLogEntriesModel : public QIdentityProxyModel {
   public:
      using underlying_model_type = QuestStagesModel;

      using Column = underlying_model_type::LogEntryColumn;
      static constexpr const size_t ColumnCount = underlying_model_type::LogEntryColumnCount;

      using LogEntryData = underlying_model_type::LogEntryData;

   public:
      using QIdentityProxyModel::QIdentityProxyModel;

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex createLogEntry(const QModelIndex& stage);
      void deleteLogEntry(const QModelIndex& log_entry);

      const LogEntryData* logEntry(const QModelIndex&) const;
      void setLogEntry(const QModelIndex&, LogEntryData&&);
      void setLogEntry(const QModelIndex&, const LogEntryData&);
};