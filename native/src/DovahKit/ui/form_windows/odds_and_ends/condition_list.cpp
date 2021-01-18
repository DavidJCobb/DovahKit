#include "condition_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../dovah/form_stub.h"
#include "../../../helpers/qt/strings.h"

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
   auto* function  = condition_function::lookup_by_id(condition.function);
   auto  flags     = condition.get_flags();
   switch (role) {
      case Qt::DisplayRole:
         switch (column) {
            case ColumnTarget:
               switch (condition.run_on) {
                  case condition::run_on_t::combat_target:
                     return tr("Combat Target", "condition list - run on");
                  case condition::run_on_t::event_data:
                     return tr("Event Data", "condition list - run on");
                  case condition::run_on_t::linked_ref:
                     return tr("Linked Ref", "condition list - run on");
                  case condition::run_on_t::package_data:
                     //
                     // TODO: check index; display which data
                     //
                     return tr("Package Data", "condition list - run on");
                  case condition::run_on_t::quest_alias:
                     return tr("Alias ID #u", "condition list - run on").arg(condition.run_on_index); // TODO: display alias name if possible
                  case condition::run_on_t::reference:
                     if (auto* stub = condition.run_on_reference.get_form_stub()) {
                        return tr("[%1:%2]%3", "condition list - run on form")
                           .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature))
                           .arg(stub->formID, 8, 16, QChar('0'))
                           .arg(stub->get_editor_id());
                     }
                     return tr("No Reference", "condition list - run on");
                  case condition::run_on_t::subject:
                     return tr("Subject", "condition list - run on");
                  case condition::run_on_t::target:
                     return tr("Target", "condition list - run on");
               }
               break;
            case ColumnFunction:
               if (function) {
                  return QString(function->name);
               }
               break;
            case ColumnArgs:
               // TODO: args (see TopicInfo screenshot for example)
               break;
            case ColumnOperator:
               switch (condition.get_operator()) {
                  case condition::operator_t::equal:
                     return tr("==", "condition list - operator, equal");
                  case condition::operator_t::greater:
                     return tr(">",  "condition list - operator, greater");
                  case condition::operator_t::greater_or_equal:
                     return tr(">=", "condition list - operator, greater or equal");
                  case condition::operator_t::less:
                     return tr("<",  "condition list - operator, less");
                  case condition::operator_t::less_or_equal:
                     return tr("<=", "condition list - operator, less or equal");
                  case condition::operator_t::not_equal:
                     return tr("!=", "condition list - operator, not equal");
               }
               break;
            case ColumnOperand:
               if (flags & condition::flag::compare_to_global) {
                  if (!condition.compare_to_global)
                     return tr("-NONE-", "condition list - compare to global (missing)");
                  auto* stub = condition.compare_to_global.get_form_stub();
                  return tr("[%1:%2]%3", "condition list - compare to global")
                     .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature))
                     .arg(stub->formID, 8, 16, QChar('0'))
                     .arg(stub->get_editor_id());
               } else {
                  return condition.compare_to_constant;
               }
               break;
            case ColumnUsesOr:
               if (flags & condition::flag::or_linked)
                  return tr("OR", "condition list - or");
               break;
         }
         break;
      case Qt::FontRole:
         switch (column) {
            case ColumnTarget:
               //
               // Show the run-on column in italics if it isn't an alias name, etc.. Italics 
               // will distinguish built-in strings from names in user content.
               //
               switch (condition.run_on) {
                  case condition::run_on_t::package_data:
                     {
                        auto font = QFont();
                        font.setItalic(true);
                        return font;
                     }
                  case condition::run_on_t::quest_alias:
                     //
                     // TODO: if it was possible to show an alias name, then (break) here.
                     //
                     [[fallthrough]]
                  case condition::run_on_t::reference:
                     if (condition.run_on_reference)
                        break;
                     [[fallthrough]]
                  case condition::run_on_t::event_data:
                  case condition::run_on_t::linked_ref:
                  case condition::run_on_t::combat_target:
                  case condition::run_on_t::subject:
                  case condition::run_on_t::target:
                     {
                        auto font = QFont();
                        font.setItalic(true);
                        return font;
                     }
               }
               break;
            case ColumnOperand:
               if (!condition.compare_to_global && (flags & condition::flag::compare_to_global)) {
                  auto font = QFont();
                  font.setItalic(true);
                  return font;
               }
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