#include "./QuestStageLogEntriesModel.h"

/*virtual*/ QVariant QuestStageLogEntriesModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return {};
   switch (section) {
      case Column::Text:
         return tr("Log Entry");
      case Column::Conditions:
         return tr("Conditions");
   }
   return {};
}

QModelIndex QuestStageLogEntriesModel::createLogEntry(const QModelIndex& stage) {
   auto* model = qobject_cast<underlying_model_type*>(this->sourceModel());
   if (!model)
      return {};
   if (stage.model() == this)
      return model->createLogEntry(this->mapToSource(stage));
   return model->createLogEntry(stage);
}
void QuestStageLogEntriesModel::deleteLogEntry(const QModelIndex& log_entry) {
   auto* model = qobject_cast<underlying_model_type*>(this->sourceModel());
   if (!model)
      return;
   if (log_entry.model() == this)
      return model->deleteLogEntry(this->mapToSource(log_entry));
   model->deleteLogEntry(log_entry);
}

const QuestStageLogEntriesModel::LogEntryData* QuestStageLogEntriesModel::logEntry(const QModelIndex& qmi) const {
   auto* model = qobject_cast<underlying_model_type*>(this->sourceModel());
   if (!model)
      return nullptr;
   return model->logEntry(qmi.model() == model ? qmi : this->mapToSource(qmi));
}
void QuestStageLogEntriesModel::setLogEntry(const QModelIndex& qmi, LogEntryData&& v) {
   auto* model = qobject_cast<underlying_model_type*>(this->sourceModel());
   if (!model)
      return;
   model->setLogEntry(qmi.model() == model ? qmi : this->mapToSource(qmi), std::move(v));
}
void QuestStageLogEntriesModel::setLogEntry(const QModelIndex& qmi, const LogEntryData& v) {
   auto* model = qobject_cast<underlying_model_type*>(this->sourceModel());
   if (!model)
      return;
   model->setLogEntry(qmi.model() == model ? qmi : this->mapToSource(qmi), v);
}