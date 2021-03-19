#include "condition_list.h"
#include <QAction>
#include <QHeaderView>
#include <QMenu>
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
#include "../../generic/QHeaderViewDKEx.h"

namespace {
   uint16_t _index_of_GetIsID() noexcept {
      constexpr auto sentinel = std::numeric_limits<uint16_t>::max();
      static uint16_t index = sentinel;
      if (index != sentinel)
         return index;
      auto& list = dovah::condition_function_list;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto& func = list[i];
         if (_stricmp(func.name, "GetIsID") == 0) {
            index = i;
            return index;
         }
      }
      index = 0;
      return index;
   }
}

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
void ConditionListModel::duplicateSelection(const QItemSelection& indices) {
   if (!this->target)
      return;
   if (!indices.size())
      return;
   //
   auto& list = *this->target;
   auto* form = this->targetLoadedForm();
   if (!form)
      return;
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int top    = range.top();
      int bottom = range.bottom();
      int diff   = bottom - top;
      this->beginInsertRows(dummy, bottom, bottom + diff);
      for (int i = bottom; i >= top; --i) {
         auto& cnd  = list[i];
         auto  wc   = cnd.make_working_copy();
         auto& dupe = *list.emplace(list.begin() + bottom + 1);
         dupe.commit(*form, wc);
      }
      this->endInsertRows();
   }
}
QModelIndex ConditionListModel::insertCondition(const dovah::loaded_forms::components::working_condition& wc, size_t at) {
   if (!this->target)
      return QModelIndex();
   auto& list = *this->target;
   at = std::min(at, list.size());
   this->beginInsertRows(QModelIndex(), at, at);
   //
   list.emplace(list.begin() + at);
   auto& cnd = list[at];
   cnd.commit(*this->targetLoadedForm(), wc);
   //
   this->endInsertRows();
   //
   return this->index(at, 0, QModelIndex());
}
void ConditionListModel::moveSelection(const QItemSelection& indices, int down) {
   if (!down || !indices.size())
      return;
   //
   auto size = this->count();
   QModelIndex dummy;
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
void ConditionListModel::removeSelection(const QItemSelection& indices) {
   if (!this->target)
      return;
   if (!indices.size())
      return;
   //
   auto& list = *this->target;
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int top    = range.top();
      int bottom = range.bottom();
      this->beginRemoveRows(dummy, top, bottom);
      list.erase(list.begin() + top, list.begin() + bottom + 1);
      this->endRemoveRows();
   }
}
void ConditionListModel::setTarget(form_stub& owner, std::vector<condition>& list, bool in_working_copy) {
   if (this->target)
      this->clearTarget();
   this->beginResetModel();
   this->target  = &list;
   this->context = cnd_context_t(owner, in_working_copy);
   this->in_working_copy = in_working_copy;
   this->endResetModel();
}

dovah::loaded_forms::Form* ConditionListModel::targetLoadedForm() const noexcept {
   if (!this->context.owner)
      return nullptr;
   if (this->in_working_copy)
      return this->context.owner->working_copy;
   return this->context.owner->form;
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
      list->setModel(new model_type(this->ui.list));
      list->sortByColumn(0, Qt::AscendingOrder);
      list->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      {
         auto* vh = list->verticalHeader();
         vh->setVisible(false);
         vh->setDefaultSectionSize(vh->minimumSectionSize());
      }
      //
      {
         auto* hdr = new QHeaderViewDKEx(Qt::Horizontal, list);
         hdr->setFlexResizeEnabled(true);
         list->setHorizontalHeader(hdr);
      }
      //
      auto header  = (QHeaderViewDKEx*)list->horizontalHeader();
      auto metrics = QFontMetrics(list->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setSortIndicatorShown(false);
      header->setColumnFlex(model_type::ColumnTarget,   2, 1, metrics.boundingRect("Target").width() * 1.5F + 4);
      header->setColumnFlex(model_type::ColumnFunction, 4, 1);
      header->setColumnFlex(model_type::ColumnOperator, 0, 0, metrics.boundingRect("==").width() * 1.5F + 4);
      header->setColumnFlex(model_type::ColumnArgs,     6, 1);
      header->setColumnFlex(model_type::ColumnOperand,  1, 1, metrics.boundingRect("9999999").width() * 1.5F + 4);
      {
         auto size_or  = metrics.boundingRect(tr("OR", "condition list - condition link - or")).width();
         auto size_and = metrics.boundingRect(tr("AND", "condition list - condition link - and")).width();
         header->setColumnFlex(model_type::ColumnUsesOr, 0, 0, std::max(size_or, size_and) * 1.5F + 4);
      }
      /*//
      header->resizeSection(model_type::ColumnTarget,   metrics.boundingRect("Target").width() * 1.5F + 4);
      header->resizeSection(model_type::ColumnFunction, metrics.boundingRect("GetVMScriptVariable").width() * 1.5F + 4);
      header->resizeSection(model_type::ColumnOperator, metrics.boundingRect("==").width() * 1.5F + 4);
      {
         auto size_or  = metrics.boundingRect(tr("OR",  "condition list - condition link - or")).width();
         auto size_and = metrics.boundingRect(tr("AND", "condition list - condition link - and")).width();
         header->resizeSection(model_type::ColumnUsesOr, std::max(size_or, size_and) * 1.5F + 4);
      }
      //*/
      for(int i = 0; i < list->model()->columnCount(); ++i)
         header->setSectionResizeMode(i, QHeaderView::Interactive);
      header->setSectionResizeMode(model_type::ColumnOperator, QHeaderView::Fixed);
      header->setSectionResizeMode(model_type::ColumnUsesOr,   QHeaderView::Fixed);
      header->setStretchLastSection(false);
   }
   QObject::connect(this->ui.list, &QTableView::doubleClicked, this, &ConditionList::openEditConditionModal);
   //
   QObject::connect(this->ui.buttonAdd, &QPushButton::clicked, this, &ConditionList::openCreateConditionModal);
   QObject::connect(this->ui.buttonMoveUp, &QPushButton::clicked, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->moveSelection(sm->selection(), -1);
      emit this->conditionEdited();
   });
   QObject::connect(this->ui.buttonMoveDown, &QPushButton::clicked, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->moveSelection(sm->selection(), 1);
      emit this->conditionEdited();
   });
   //
   #pragma region Context menu
   this->context_menu_actions.create    = new QAction(tr("New..."));
   this->context_menu_actions.edit      = new QAction(tr("Edit..."));
   this->context_menu_actions.duplicate = new QAction(tr("Duplicate"));
   this->context_menu_actions.destroy   = new QAction(tr("Delete"));
   //
   QObject::connect(this->context_menu_actions.create,    &QAction::triggered, this, &ConditionList::openCreateConditionModal);
   QObject::connect(this->context_menu_actions.edit,      &QAction::triggered, this, &ConditionList::openEditConditionModal);
   QObject::connect(this->context_menu_actions.duplicate, &QAction::triggered, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->duplicateSelection(sm->selection());
      sm->select(QItemSelection(), QItemSelectionModel::ClearAndSelect); // clear the selection
      emit this->conditionEdited();
   });
   QObject::connect(this->context_menu_actions.destroy, &QAction::triggered, this, [this]() {
      auto* model = this->model();
      auto* sm    = this->ui.list->selectionModel();
      if (!model || !sm)
         return;
      model->removeSelection(sm->selection());
      emit this->conditionEdited();
   });
   //
   this->ui.list->setContextMenuPolicy(Qt::CustomContextMenu);
   QObject::connect(this->ui.list, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
      auto* opener = this->ui.list;
      auto* sm     = opener->selectionModel();
      if (!sm)
         return;
      auto rows = sm->selectedRows();
      //
      this->context_menu_actions.edit->setVisible(rows.size() == 1);
      this->context_menu_actions.duplicate->setVisible(!rows.isEmpty());
      //
      QMenu menu(opener);
      menu.addAction(this->context_menu_actions.create);
      menu.addAction(this->context_menu_actions.edit);
      menu.addAction(this->context_menu_actions.duplicate);
      menu.addAction(this->context_menu_actions.destroy);
      //
      if (menu.isEmpty())
         return; // don't show a menu if all of its contents are disabled or hidden
      menu.exec(opener->mapToGlobal(pos));
   });
   #pragma endregion
}
ConditionList::model_type* ConditionList::model() const noexcept {
   return (model_type*) this->ui.list->model();
}
void ConditionList::openCreateConditionModal() {
   auto* model = this->model();
   auto* sm    = this->ui.list->selectionModel();
   if (!model || !sm)
      return;
   auto* stub = model->targetStub();
   if (!stub)
      return;
   size_t insert_at = std::numeric_limits<size_t>::max();
   auto   rows      = sm->selectedRows();
   if (!rows.isEmpty())
      insert_at = rows.back().row() + 1;
   {
      using _wc_t = dovah::loaded_forms::components::working_condition;
      //
      // We need to heap-allocate the working condition in order to ensure that it persists 
      // past the end of this function call, remaining available to the "accepted" event 
      // lambda.
      //
      auto* work = new _wc_t();
      work->function      = _index_of_GetIsID();
      work->run_on.type   = _wc_t::run_on_type::subject;
      work->comparison.op = _wc_t::operator_type::equal;
      work->comparison.operand.constant = 1.0F;
      work->reset_parameters();
      //
      auto* modal = new ConditionEditDialog(*stub, *work, this);
      QObject::connect(modal, &QDialog::accepted, this, [this, model, work, insert_at]() {
         auto  qmi = model->insertCondition(*work, insert_at);
         auto* sm  = this->ui.list->selectionModel();
         if (!sm)
            return;
         QModelIndex    dummy;
         QModelIndex    br = model->index(qmi.row(), model->columnCount(dummy) - 1, dummy); // (qmi) is just the left "edge" of the selection, and we want to select the whole row
         QItemSelection range(qmi, br);
         sm->select(range, QItemSelectionModel::ClearAndSelect);
         emit this->conditionEdited();
      });
      QObject::connect(modal, &QDialog::finished, this, [work](int code) {
         //
         // This doesn't seem to be formally specified in the documentation but as of my 
         // last look at QDialog's source code, (accepted) or (rejected) are fired before 
         // (finished), so this should be safe.
         //
         delete work;
      });
      modal->open();
   }
}
void ConditionList::openEditConditionModal() {
   auto* model = this->model();
   auto* sm    = this->ui.list->selectionModel();
   if (!model || !sm)
      return;
   auto  rows = sm->selectedRows();
   if (rows.size() != 1)
      return;
   auto  qmi  = rows[0];
   auto* stub = model->targetStub();
   auto* cnd  = model->getCondition(qmi);
   if (!stub || !cnd)
      return;
   {
      //
      // We need to heap-allocate the working condition in order to ensure that it persists 
      // past the end of this function call, remaining available to the "accepted" event 
      // lambda.
      //
      auto* work = new dovah::loaded_forms::components::working_condition();
      *work = cnd->make_working_copy();
      //
      auto* modal = new ConditionEditDialog(*stub, *work, this);
      QObject::connect(modal, &QDialog::accepted, this, [this, model, qmi, cnd, work]() {
         auto* form = model->targetLoadedForm();
         if (!form)
            return;
         cnd->commit(*form, *work);
         emit model->dataChanged(qmi, qmi);
         emit this->conditionEdited();
      });
      QObject::connect(modal, &QDialog::finished, this, [work](int code) {
         //
         // This doesn't seem to be formally specified in the documentation but as of my 
         // last look at QDialog's source code, (accepted) or (rejected) are fired before 
         // (finished), so this should be safe.
         //
         delete work;
      });
      modal->open();
   }
}
#pragma endregion