#include "condition_list.h"
#include <QHeaderView>
#include "../../../editor/core.h"
#include "../../../editor/helpers/stringify_condition_argument.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Form.h"
#include "../../../dovah/forms/Quest.h"
#include "../../../dovah/forms/factories/hardcoded.h"
#include "../../../dovah/data/story_manager.h"
#include "../../../helpers/qt/strings.h"
#include "../../../helpers/vector.h"
#include "condition_edit.h"

#pragma region ConditionListModel
ConditionListModel::ConditionListModel(QObject* parent) : QAbstractTableModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &ConditionListModel::clearTarget);
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
      if (condition.refers_to_form(stub)) {
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
   auto& condition  = (*this->target)[row];
   auto* function   = condition.get_function();
   auto  flags      = condition.get_flags();
   auto& comparison = condition.get_comparison();
   switch (column) {
      case ColumnTarget:
         switch (role) {
            case Qt::DisplayRole:
               [[fallthrough]];
            case Qt::ToolTipRole:
               switch (condition.get_run_on_data().type) {
                  case condition::run_on_type::combat_target:
                     return tr("Combat Target", "condition list - run on");
                  case condition::run_on_type::event_data:
                     if (auto* q = this->context.get_owning_quest()) {
                        auto  code = q->event;
                        auto* def  = dovah::story_event_definition::lookup(code);
                        if (def) {
                           auto* member = def->member_by_wide_signature(condition.get_run_on_data().index);
                           if (member)
                              return tr("Event Data: %1", "condition list - run on").arg(member->name);
                        }
                     }
                     return tr("Event Data", "condition list - run on");
                  case condition::run_on_type::linked_ref:
                     return tr("Linked Ref", "condition list - run on");
                  case condition::run_on_type::package_data:
                     //
                     // TODO: check index; display which data
                     //
                     return tr("Package Data", "condition list - run on");
                  case condition::run_on_type::quest_alias:
                     if (auto* q = this->context.get_owning_quest()) {
                        if (auto* alias = q->lookup_alias_by_id(condition.get_run_on_data().index)) {
                           QString name = alias->name.c_str();
                           if (!name.trimmed().isEmpty())
                              return name;
                        }
                     }
                     return tr("Alias ID #%1", "condition list - run on").arg(condition.get_run_on_data().index); // TODO: display alias name if possible
                  case condition::run_on_type::reference:
                     if (auto* stub = condition.get_run_on_data().reference.get_form_stub()) {
                        if (stub->formID == dovah::hardcoded_form_ids::PlayerRef)
                           return tr("Player", "condition list - run on form - player");
                        return tr("[%1:%2]%3", "condition list - run on form")
                           .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature))
                           .arg(stub->formID, 8, 16, QChar('0'))
                           .arg(stub->get_editor_id());
                     }
                     return tr("No Reference", "condition list - run on");
                  case condition::run_on_type::subject:
                     return tr("Subject", "condition list - run on");
                  case condition::run_on_type::target:
                     return tr("Target", "condition list - run on");
               }
               break;
            case Qt::FontRole:
               //
               // Show the run-on column in italics if it isn't an alias name, etc.. Italics 
               // will distinguish built-in strings from names in user content.
               //
               switch (condition.get_run_on_data().type) {
                  case condition::run_on_type::package_data:
                     {
                     // TODO: revisit this when we actually know what package data indices *are*
                        auto font = QFont();
                        font.setItalic(true);
                        return font;
                     }
                  case condition::run_on_type::quest_alias:
                     if (auto* q = this->context.get_owning_quest()) {
                        if (auto* alias = q->lookup_alias_by_id(condition.get_run_on_data().index)) {
                           QString name = alias->name.c_str();
                           if (!name.trimmed().isEmpty())
                              break;
                        }
                     }
                     [[fallthrough]];
                  case condition::run_on_type::reference:
                     if (condition.get_run_on_data().reference)
                        break;
                     [[fallthrough]];
                  case condition::run_on_type::event_data:
                  case condition::run_on_type::linked_ref:
                  case condition::run_on_type::combat_target:
                  case condition::run_on_type::subject:
                  case condition::run_on_type::target:
                     {
                        auto font = QFont();
                        font.setItalic(true);
                        return font;
                     }
               }
               break;
         }
         break;
      case ColumnFunction:
         switch (role) {
            case Qt::DisplayRole:
               [[fallthrough]];
            case Qt::ToolTipRole:
               if (function)
                  return QString(function->name);
               break;
            case Qt::ForegroundRole:
               if (false) // TODO: Bethesda's Creation Kit hardcodes specific conditions to show up in purple; look for "editorFilter" in CommandTable defs
                  return QColor::fromRgb(0x800080);
               break;
         }
         break;
      case ColumnArgs:
         switch (role) {
            case Qt::DisplayRole:
               [[fallthrough]];
            case Qt::ToolTipRole:
               if (!function)
                  break;
               if (function->argument_types[0] != &dovah::condition_parameter_types::None) {
                  bool dummy;
                  auto value_a = editor_helpers::stringify_condition_argument(dummy, condition, 0, this->context);
                  if (function->argument_types[1] != &dovah::condition_parameter_types::None) {
                     auto value_b = editor_helpers::stringify_condition_argument(dummy, condition, 1, this->context);
                     return tr("%1, %2").arg(value_a).arg(value_b);
                  }
                  return value_a;
               }
               break;
         }
         break;
      case ColumnOperator:
         if (role == Qt::DisplayRole) {
            switch (comparison.op) {
               case condition::operator_type::equal:
                  return tr("==", "condition list - operator, equal");
               case condition::operator_type::greater:
                  return tr(">",  "condition list - operator, greater");
               case condition::operator_type::greater_or_equal:
                  return tr(">=", "condition list - operator, greater or equal");
               case condition::operator_type::less:
                  return tr("<",  "condition list - operator, less");
               case condition::operator_type::less_or_equal:
                  return tr("<=", "condition list - operator, less or equal");
               case condition::operator_type::not_equal:
                  return tr("!=", "condition list - operator, not equal");
            }
         }
         break;
      case ColumnOperand:
         switch (role) {
            case Qt::DisplayRole:
               [[fallthrough]];
            case Qt::ToolTipRole:
               if (flags & condition::flag::compare_to_global) {
                  if (!comparison.operand.global)
                     return tr("NONE", "condition list - compare to global (missing)");
                  auto* stub = comparison.operand.global.get_form_stub();
                  return tr("[%1:%2]%3", "condition list - compare to global")
                     .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->formType).signature))
                     .arg(stub->formID, 8, 16, QChar('0'))
                     .arg(stub->get_editor_id());
               } else {
                  return comparison.operand.constant;
               }
               break;
            case Qt::FontRole:
               if (!comparison.operand.global && (flags & condition::flag::compare_to_global)) {
                  auto font = QFont();
                  font.setItalic(true);
                  return font;
               }
               break;
         }
         break;
      case ColumnUsesOr:
         switch (role) {
            case Qt::DisplayRole:
               if (flags & condition::flag::or_linked)
                  return tr("OR", "condition list - condition link - or");
               return tr("AND", "condition list - condition link - and");
         }
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
bool ConditionListModel::moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) {
   if (!this->target)
      return false;
   auto& list = *this->target;
   auto  size = list.size();
   //
   // Correctness checks:
   //
   if (first_row_index < 0 || count <= 0 || to_position < 0) // don't move anything to/from before the start of the table
      return false;
   if (to_position > size) // don't move anything past the end of the table
      return false;
   auto last_to_move = first_row_index + count - 1;
   if (last_to_move >= size) // don't move anything past the end of the table
      return false;
   //
   // Let Qt run its own correctness checks and then run its preparations. Among other things, 
   // this should check to make sure that the target position isn't in the middle of the range of 
   // rows you're moving.
   //
   if (!beginMoveRows(from_parent, first_row_index, last_to_move, to_parent, to_position))
      return false;
   //
   if (to_position >= first_row_index) { // are we moving elements down?
      //
      // Qt's API design is such that (to_position) is always the position at which the first 
      // of the moved rows will end up, unless you're moving rows down within the same parent, 
      // in which case (to_position) is the position before which the last of the moved rows 
      // will end up. Here, we normalize it to always be the position at which the first of 
      // the moved rows will end up.
      //
      to_position -= count;
   }
   cobb::move_range(list, first_row_index, count, to_position); // TODO: WE AREN'T MOVING DOWN PROPERLY. TEST IS DialogueCrimeGuards
   //
   // And we're done!
   //
   endMoveRows();
   return true;
}

void ConditionListModel::clearTarget() {
   this->beginResetModel();
   this->target  = nullptr;
   this->context = cnd_context_t();
   this->endResetModel();
}
void ConditionListModel::moveSelection(const QItemSelection& indices, int down) {
   if (!down || !indices.size())
      return;
   //
   QModelIndex dummy;
   auto size = this->count();
   for (const QItemSelectionRange& range : indices) {
      int to;
      int top    = range.top();
      int bottom = range.bottom();
      if (down < 0) {
         if (top < -down)
            continue;
         to = top + down;
      } else if (down > 0) {
         if (bottom + down >= size)
            continue;
         //
         // Typically, when moving rows, the "destination index" is the index that the 
         // first of the moved rows will be placed at. However, when moving rows down 
         // within the same parent, the "destination index" is the index that the last 
         // row will be placed before.
         //
         to = bottom + down + 1;
      }
      this->moveRows(dummy, top, bottom - top + 1, dummy, to);
   }
}
void ConditionListModel::refresh() {
   if (!this->target)
      return;
   QModelIndex dummy;
   auto last = this->target->size() - 1;
   emit dataChanged(this->index(0, 0, dummy), this->index(last, this->columnCount(dummy), dummy));
}
void ConditionListModel::setTarget(form_stub& owner, std::vector<condition>& list) {
   if (this->target)
      this->clearTarget();
   this->beginResetModel();
   this->target  = &list;
   this->context = cnd_context_t(owner);
   this->endResetModel();
}

ConditionListModel::condition* ConditionListModel::getCondition(const QModelIndex& index) {
   if (!this->target)
      return nullptr;
   auto r = index.row();
   if (r >= 0 && r <= this->target->size()) {
      return &(*this->target)[r];
   }
   return nullptr;
}
#pragma endregion

#pragma region ConditionList
ConditionList::ConditionList(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   //
   {
      auto* list = this->ui.list;
      //
      list->setModel(new model_type);
      list->sortByColumn(0, Qt::AscendingOrder);
      list->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      {
         auto* vh = list->verticalHeader();
         vh->setVisible(false);
         vh->setDefaultSectionSize(vh->minimumSectionSize());
      }
      //
      auto header  = list->horizontalHeader();
      auto metrics = QFontMetrics(list->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setSortIndicatorShown(false);
      header->resizeSection(model_type::ColumnTarget,   metrics.boundingRect("Target").width() * 1.5F + 4);
      header->resizeSection(model_type::ColumnFunction, metrics.boundingRect("GetVMScriptVariable").width() * 1.5F + 4);
      header->resizeSection(model_type::ColumnOperator, metrics.boundingRect("==").width() * 1.5F + 4);
      {
         auto size_or  = metrics.boundingRect(tr("OR",  "condition list - condition link - or")).width();
         auto size_and = metrics.boundingRect(tr("AND", "condition list - condition link - and")).width();
         header->resizeSection(model_type::ColumnUsesOr, std::max(size_or, size_and) * 1.5F + 4);
      }
      for(int i = 0; i < list->model()->columnCount(); ++i)
         header->setSectionResizeMode(i, QHeaderView::Interactive);
      header->setSectionResizeMode(model_type::ColumnOperator, QHeaderView::Fixed);
      header->setSectionResizeMode(model_type::ColumnUsesOr,   QHeaderView::Fixed);
      header->setStretchLastSection(false);
   }
   QObject::connect(this->ui.list, &QTableView::doubleClicked, [this](const QModelIndex& index) {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      auto  rows = sm->selectedRows();
      if (rows.size() != 1)
         return;
      auto* stub = model->targetStub();
      auto* cnd  = model->getCondition(rows[0]);
      if (!stub || !cnd)
         return;
      auto* modal = new ConditionEditDialog(*stub, *cnd, this);
      modal->open();
   });
   //
   QObject::connect(this->ui.buttonAdd, &QPushButton::clicked, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      //
      // TODO: insert new condition after last selected condition
      //
   });
   QObject::connect(this->ui.buttonMoveUp, &QPushButton::clicked, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->moveSelection(sm->selection(), -1);
   });
   QObject::connect(this->ui.buttonMoveDown, &QPushButton::clicked, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->moveSelection(sm->selection(), 1);
   });
}
ConditionList::model_type* ConditionList::model() const noexcept {
   return (model_type*) this->ui.list->model();
}
#pragma endregion