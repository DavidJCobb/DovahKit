#include "condition_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"

#pragma region ConditionListModel
ConditionListModel::ConditionListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &ConditionListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &ConditionListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &ConditionListModel::formDeletionImminent);
}

void ConditionListModel::formModified(const dovah::form_stub* stub) {
   if (!this->target)
      return;
   auto parent = QModelIndex();
   auto column = this->columnCount(parent);
   auto size   = this->target->size();
   for (size_t i = 0; i < size; ++i) {
      auto& condition = (*this->target)[i];
      //
      bool update = false;
      for (int j = 0; j < 2; ++j) {
         auto ud = condition.get_argument_underlying_type(j);
         if (ud == arg_underlying_type::aliasID) {
            if (stub == this->owner) {
               update = true;
               break;
            }
         } else if (ud == arg_underlying_type::formID) {
            if (condition.parameters[j].form == stub) {
               update = true;
               break;
            }
         }
      }
      //
      if (update) {
         auto start = this->index(i, 0, parent);
         auto end   = this->index(i, column, parent);
         emit dataChanged(start, end);
      }
   }
}
void ConditionListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   //
   // TODO: we want to have DovahKitCore "wrap" the temporary form clone so that it 
   // automatically reacts to this, so how do we ensure that this model repaints 
   // after that occurs?
   //
}

QModelIndex ConditionListModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return QModelIndex();
   if (!this->target)
      return QModelIndex();
   if (row >= 0 && row < this->target->size())
      return this->createIndex(row, column, nullptr);
   return QModelIndex();
}
QModelIndex ConditionListModel::parent(const QModelIndex& index) const {
   return QModelIndex();
}
int ConditionListModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   if (!this->target)
      return 0;
   return this->target->size();
}
int ConditionListModel::columnCount(const QModelIndex& item) const {
   return 6;
}
Qt::ItemFlags ConditionListModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::NoItemFlags;
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
QVariant ConditionListModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid())
      return QVariant();
   auto row     = index.row();
   auto column  = index.column();
   if (!this->target)
      return QVariant();
   auto& condition = (*this->target)[row];
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case ColumnTarget:
               // TODO: run-on
               break;
            case ColumnFunction:
               // TODO: function name
               break;
            case ColumnArgs:
               // TODO: args (see TopicInfo screenshot for example)
               break;
            case ColumnOperator:
               // TODO: comparison operator
               break;
            case ColumnOperand:
               // TODO: float or global
               break;
            case ColumnUsesOr:
               if (condition.get_flags() & condition::flag::or_linked)
                  return tr("OR", "condition list - or");
               break;
         }
         break;
      case Qt::ForegroundRole:
         if (false) // TODO: Bethesda's Creation Kit hardcodes specific conditions to show up in purple; look for "editorFilter" in CommandTable defs
            return QColor::fromRgb(0x800080);
         break;
   }
   return QVariant();
}
//
QVariant ConditionListModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation != Qt::Orientation::Horizontal)
      return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         switch (section) {
            case ColumnTarget:   return tr("Target",    "condition list");
            case ColumnFunction: return tr("Function",  "condition list");
            case ColumnArgs:     return tr("Arguments", "condition list");
            case ColumnOperator: return tr("Operator",  "condition list");
            case ColumnOperand:  return tr("Operand",   "condition list");
            case ColumnUsesOr:   return tr("",          "condition list (uses-OR col header)");
         }
         break;
   }
   return QVariant();
}

void ConditionListModel::clear() {
   this->beginResetModel();
   if (this->target)
      this->target->clear();
   this->endResetModel();
}
#pragma endregion

#pragma region ConditionList
ConditionList::ConditionList(QWidget* parent) : QTableView(parent) {
   this->setModel(new model_type);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(0, Qt::AscendingOrder);
   this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(model_type::ColumnTarget,   metrics.boundingRect("Target").width() * 1.5F + 4);
   header->resizeSection(model_type::ColumnFunction, metrics.boundingRect("GetVMScriptVariable").width() * 1.5F + 4);
   header->resizeSection(model_type::ColumnOperator, metrics.boundingRect("==").width() * 1.5F + 4);
   header->resizeSection(model_type::ColumnUsesOr,   metrics.boundingRect("OR").width() * 1.5F + 4);
   for(int i = 0; i < this->model()->columnCount(); ++i)
      header->setSectionResizeMode(i, QHeaderView::Interactive);
};
#pragma endregion