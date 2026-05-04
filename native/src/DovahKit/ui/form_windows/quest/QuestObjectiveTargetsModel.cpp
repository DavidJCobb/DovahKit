#include "./QuestObjectiveTargetsModel.h"

/*virtual*/ QVariant QuestObjectiveTargetsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return {};
   switch (section) {
      case Column::AliasName:
         return tr("Target Alias");
      case Column::Conditions:
         return tr("Conditions");
   }
   return {};
}

QModelIndex QuestObjectiveTargetsModel::createTarget(const QModelIndex& objective) {
   auto* model = qobject_cast<QuestObjectivesModel*>(this->sourceModel());
   if (!model)
      return {};
   if (objective.model() == this)
      return model->createTarget(this->mapToSource(objective));
   return model->createTarget(objective);
}
void QuestObjectiveTargetsModel::deleteTarget(const QModelIndex& target) {
   auto* model = qobject_cast<QuestObjectivesModel*>(this->sourceModel());
   if (!model)
      return;
   if (target.model() == this)
      return model->deleteTarget(this->mapToSource(target));
   model->deleteTarget(target);
}

QuestObjectiveTargetsModel::condition_list QuestObjectiveTargetsModel::targetConditions(const QModelIndex& qmi) const {
   auto* model = qobject_cast<QuestObjectivesModel*>(this->sourceModel());
   if (!model)
      return {};
   return model->targetConditions(qmi.model() == this ? this->mapToSource(qmi) : qmi);
}
void QuestObjectiveTargetsModel::setTargetConditions(const QModelIndex& qmi, condition_list&& list) {
   auto* model = qobject_cast<QuestObjectivesModel*>(this->sourceModel());
   if (!model)
      return;
   model->setTargetConditions(qmi.model() == this ? this->mapToSource(qmi) : qmi, std::move(list));
}