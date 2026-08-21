#include "./DKConditionListModel.h"
#include <QColor>
#include <QFont>
#include <QIODevice>
#include <QMimeData>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/Form.h"
#include "editor/core.h"
#include "editor/helpers/condition_to_string/boolean_link.h"
#include "editor/helpers/condition_to_string/comparison_operand.h"
#include "editor/helpers/condition_to_string/comparison_operator.h"
#include "editor/helpers/condition_to_string/parameter_set.h"
#include "editor/helpers/condition_to_string/run_on.h"
#include "editor/helpers/condition_mime_data.h"
#include "editor/helpers/stringify_conditions.h" // for plain-text copy/paste

namespace {
   namespace special_case_functions {
      constexpr const auto _lookup_function_id_by_name(std::string_view name) {
         for (const auto& info : dovah::conditions::all_vanilla_function_info)
            if (info.name == name)
               return info.id;
         throw;
      }

      constexpr const auto GetVMQuestVariable    = _lookup_function_id_by_name("GetVMQuestVariable");
      constexpr const auto GetVMScriptVariable   = _lookup_function_id_by_name("GetVMScriptVariable");
      constexpr const auto IsInCombat            = _lookup_function_id_by_name("IsInCombat");
      constexpr const auto IsLimbGone            = _lookup_function_id_by_name("IsLimbGone");
      constexpr const auto IsPlayerActionActive  = _lookup_function_id_by_name("IsPlayerActionActive");
      constexpr const auto IsSceneActionComplete = _lookup_function_id_by_name("IsSceneActionComplete");
   }

   constexpr const char* const conditions_binary_mime_type = "application/dovah-kit.form-condition-array";
}

DKConditionListModel::DKConditionListModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &DKConditionListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &DKConditionListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DKConditionListModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::questWorkingCopyAliasesAltered,       this, &DKConditionListModel::handleContextChange);
   QObject::connect(&editor, &DovahKitCore::packageWorkingCopyPackageDataAltered, this, &DKConditionListModel::handleContextChange);
}
DKConditionListModel::~DKConditionListModel() {
   this->clear();
}
      
void DKConditionListModel::formModified(const dovah::form_stub* stub) {
   QModelIndex parent;

   auto column = this->columnCount(parent);
   auto size   = this->_nodes.size();
   for (size_t i = 0; i < size; ++i) {
      auto* condition = this->_nodes[i];
      if (condition->refers_to_form(stub)) {
         auto start = this->index(i, 0, parent);
         auto end   = this->index(i, column, parent);
         emit dataChanged(start, end);
      }
   }
}
void DKConditionListModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   QModelIndex parent;

   auto column = this->columnCount(parent);
   auto size   = this->_nodes.size();
   for (size_t i = 0; i < size; ++i) {
      auto* condition = this->_nodes[i];
      if (condition->sever_outbound_references_to(stub)) {
         auto start = this->index(i, 0, parent);
         auto end   = this->index(i, column, parent);
         emit dataChanged(start, end);
      }
   }
}
void DKConditionListModel::handleContextChange() {
   this->_context.update_from_owning_package();

   auto size = this->_nodes.size();
   auto tl = this->index(0, 0, {});
   auto br = this->index(size - 1, Column::_COUNT - 1, {});
   emit dataChanged(tl, br);
}

QVariant DKConditionListModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   const auto* function = dovah::conditions::function_info_by_id(node.function);

   switch (column) {
      case Column::Target:
         {
            uint32_t          index = -1;
            dovah::form_stub* stub  = nullptr;
            if (std::holds_alternative<uint32_t>(node.run_on.entity)) {
               index = std::get<uint32_t>(node.run_on.entity);
            } else if (std::holds_alternative<dovah::form_stub*>(node.run_on.entity)) {
               stub = std::get<dovah::form_stub*>(node.run_on.entity);
            }

            auto text_and_type = editor_helpers::condition_to_string::run_on(
               this->_context,
               node.run_on.type,
               index,
               stub
            );

            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               return text_and_type.first;
            } else if (role == Qt::FontRole) {
               if (text_and_type.second) {
                  QFont italics;
                  italics.setItalic(true);
                  return italics;
               }
            }
         }
         break;
         
      case Column::Function:
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            if (function)
               return QString::fromUtf8(QByteArray(function->name.data(), function->name.size()));
         } else if (role == Qt::ForegroundRole) {
            if (false) // TODO: Bethesda's Creation Kit hardcodes specific conditions to show up in purple; look for "editorFilter" in CommandTable defs
               return QColor::fromRgb(0x800080);
         }
         break;
      case Column::Args:
         switch (role) {
            case Qt::DisplayRole:
               [[fallthrough]];
            case Qt::ToolTipRole:
               return editor_helpers::condition_to_string::parameter_set(this->_context, node, true, true, {
                  .form_type = editor_helpers::condition_to_string::options::form_type_format::name,
               });
         }
         break;
      case Column::Operator:
         if (role == Qt::DisplayRole) {
            return editor_helpers::condition_to_string::comparison_operator(node.comparison.op);
         }
         break;
      case Column::Operand:
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            return editor_helpers::condition_to_string::comparison_operand(node.comparison);
         } else if (role == Qt::FontRole) {
            if (std::holds_alternative<dovah::form_stub*>(node.comparison.operand)) {
               QFont italics;
               italics.setItalic(true);
               return italics;
            }
         }
         break;
      case Column::UsesOr:
         if (role == Qt::DisplayRole) {
            return editor_helpers::condition_to_string::boolean_link(node.flags.or_linked);
         }
         break;
   }

   return {};
}
Qt::ItemFlags DKConditionListModel::flags_of(const node_type& node, size_t column) const {
   Qt::ItemFlags flags = Qt::ItemFlag::ItemIsSelectable;
   if (this->_allow_modifications_from == 0) {
      flags |= Qt::ItemFlag::ItemIsEnabled;
   } else {
      bool enabled = true;

      assert(this->_allow_modifications_from < this->_nodes.size());
      for (size_t i = 0; i < this->_allow_modifications_from; ++i) {
         if (this->_nodes[i] == &node) {
            enabled = false;
            break;
         }
      }
      if (enabled)
         flags |= Qt::ItemFlag::ItemIsEnabled;
   }
   return flags;
}
      
void DKConditionListModel::clear() {
   DKGenericListModel::clear();
   this->_context = {};
   this->_allow_modifications_from = 0;
}

/*virtual*/ QVariant DKConditionListModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
   if (orientation != Qt::Orientation::Horizontal)
      return {};
   if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
      return {};

   constexpr const char* disambig = "column header";
   switch (section) {
      case Column::Target:   return tr("Target", disambig);
      case Column::Function: return tr("Function", disambig);
      case Column::Args:     return tr("Arguments", disambig);
      case Column::Operator: return tr("Operator", disambig);
      case Column::Operand:  return tr("Operand", disambig);
      case Column::UsesOr:   return tr("", "column header (and/or linkage)");
   }

   return {};
}
/*virtual*/ QMimeData* DKConditionListModel::mimeData(const QModelIndexList& indexes) const /*override*/ {
   QMimeData* out = new QMimeData;
   {  // plaintext
      QString text;
      for (const auto& qmi : indexes) {
         const int row = qmi.row();
         if (row < 0 || row >= this->_nodes.size())
            continue;
         const auto& node = *this->_nodes[row];
         text += editor_helpers::stringify_condition(node, this->_context);
         text += ' ';
         text += editor_helpers::stringify_condition_boolean_operator(node);
         if (row + 1 < this->_nodes.size())
            text += '\n';
      }
      out->setText(text);
   }
   {  // binary
      QByteArray  data;
      QDataStream stream(&data, QIODevice::WriteOnly);

      auto _stream_form = [&stream](dovah::form_stub* stub) {
         uint32_t form_id = 0;
         if (stub)
            form_id = stub->formID;
         stream << form_id;
      };

      for (const auto& qmi : indexes) {
         const int row = qmi.row();
         if (row < 0 || row >= this->_nodes.size())
            continue;
         const auto& node = *this->_nodes[row];
         editor_helpers::append_condition_to_mime_data_stream(stream, node);
      }
      out->setData(editor_helpers::form_condition_array_mime_type, data);
   }

   return out;
}
/*virtual*/ QStringList DKConditionListModel::mimeTypes() const /*override*/ {
   return {
      "text/plain",
      editor_helpers::form_condition_array_mime_type
   };
}
/*virtual*/ bool DKConditionListModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/ {
   if (!data->hasFormat(editor_helpers::form_condition_array_mime_type))
      return false;
   return true;
}
/*virtual*/ bool DKConditionListModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
   if (!this->canDropMimeData(data, action, row, column, parent))
      return false;
   if (action == Qt::IgnoreAction)
      return true;
   if (row == -1) {
      if (parent.isValid())
         row = parent.row();
      else
         row = this->_nodes.size();
   }
   
   auto dropped = editor_helpers::conditions_from_mime_data(*data);
   this->_nodes.reserve(this->_nodes.size() + dropped.size());
   this->beginInsertRows({}, row, row + dropped.size() - 1);
   for (size_t i = 0; i < dropped.size(); ++i) {
      auto  it   = this->_nodes.insert(this->_nodes.begin() + row + i, nullptr);
      auto& ptr  = *it;
      auto& node = *(ptr = new node_type);
      node = std::move(dropped[i]);
   }
   this->endInsertRows();
   return true;
}
/*virtual*/ Qt::DropActions DKConditionListModel::supportedDropActions() const /*override*/ {
   return Qt::CopyAction;
}

[[nodiscard]] const std::vector<DKConditionListModel::Condition> DKConditionListModel::conditions() const noexcept {
   std::vector<Condition> out;
   out.reserve(this->_nodes.size());
   for (auto* node : this->_nodes) {
      out.push_back(*node);
   }
   return out;
}
void DKConditionListModel::duplicate(const QItemSelection& indices) {
   if (!indices.size())
      return;
   
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int top    = range.top();
      int bottom = range.bottom();
      int at     = bottom;
      int diff   = bottom - top;
      if (at < this->_allow_modifications_from) {
         //
         // If some of the conditions we're duplicating are in the locked range, 
         // ensure that we duplicate them below that range.
         //
         at = this->_allow_modifications_from;
      }
      this->beginInsertRows(dummy, at, at + diff);
      for (int i = bottom; i >= top; --i) {
         auto* clone = new node_type{ *this->_nodes[i] };
         this->_nodes.insert(this->_nodes.begin() + at + 1, clone);
      }
      this->endInsertRows();
   }
}
QModelIndex DKConditionListModel::insertAt(const Condition& data, size_t at) {
   if (at >= this->_nodes.size())
      at = this->_nodes.size();
   if (at < this->_allow_modifications_from)
      //
      // Do not allow insertions into the locked range.
      //
      at = this->_allow_modifications_from;

   this->beginInsertRows({}, at, at);
   auto* clone = new node_type{ data };
   this->_nodes.insert(this->_nodes.begin() + at, clone);
   this->endInsertRows();

   return this->index(at, 0, {});
}
void DKConditionListModel::move(const QItemSelection& indices, int down) {
   if (!down || !indices.size())
      return;
   //
   auto size = this->conditionCount();
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int to;
      int top    = range.top();
      int bottom = range.bottom();
      if (top < this->_allow_modifications_from) {
         //
         // Some of the conditions we wish to move are locked. Omit them from 
         // the move operation.
         //
         if (bottom < this->_allow_modifications_from)
            //
            // Actually, all of the conditions we wish to move are locked. 
            // Skip this move operation.
            //
            continue;
      }
      if (down < 0) {
         if (top < -down)
            continue;
         to = top + down;

         if (to < this->_allow_modifications_from) {
            //
            // We're trying to move the conditions into the locked range. Stop 
            // just short of it.
            //
            to = this->_allow_modifications_from;
            if (top == to)
               //
               // We can't move them any further.
               //
               continue;
         }
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
void DKConditionListModel::remove(const QItemSelection& indices) {
   if (!indices.size())
      return;
   
   auto& list = this->_nodes;
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int top    = range.top();
      int bottom = range.bottom();
      if (top < this->_allow_modifications_from) {
         //
         // Some of the conditions we're trying to edit are locked. Omit them from 
         // the removal operation.
         //
         if (bottom < this->_allow_modifications_from)
            //
            // Actually, all of the conditions we wish to remove are locked. Skip 
            // this removal operation.
            //
            continue;
         top = this->_allow_modifications_from;
      }
      this->beginRemoveRows(dummy, top, bottom);
      list.erase(list.begin() + top, list.begin() + bottom + 1);
      this->endRemoveRows();
   }
}

const DKConditionListModel::Condition* DKConditionListModel::getCondition(size_t row) const {
   if (row >= this->_nodes.size())
      return nullptr;
   return this->_nodes[row];
}
void DKConditionListModel::setCondition(size_t row, const Condition& src) {
   if (row >= this->_nodes.size())
      return;
   if (row < this->_allow_modifications_from)
      //
      // Do not allow overwriting conditions in the locked range.
      //
      return;
   *this->_nodes[row] = src;

   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
}

// Returns number of invalid conditions discarded.
size_t DKConditionListModel::importFrom(dovah::loaded_forms::Form& src_form, const BackendConditionList& src) {
   size_t invalid = 0;
   this->performReset([this, &src_form, &src, &invalid]() {
      this->_allow_modifications_from = 0;

      size_t size = src.size();
      this->_nodes.reserve(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = new node_type{ src[i] };
         if (!node->valid()) {
            ++invalid;
            delete node;
            continue;
         }
         this->_nodes.push_back(node);
      }
      this->_context = ui::types::conditions::context(src_form.stub, src_form.is_working_copy);
   });
   return invalid;
}
size_t DKConditionListModel::importFrom(dovah::loaded_forms::Form& src_form, const std::vector<Condition>& src) {
   size_t invalid = 0;
   this->performReset([this, &src_form, &src, &invalid]() {
      this->_allow_modifications_from = 0;

      size_t size = src.size();
      this->_nodes.reserve(size);
      for (size_t i = 0; i < size; ++i) {
         auto* node = new node_type{ src[i] };
         if (!node->valid()) {
            ++invalid;
            delete node;
            continue;
         }
         this->_nodes.push_back(node);
      }
      this->_context = ui::types::conditions::context(src_form.stub, src_form.is_working_copy);
   });
   return invalid;
}

void DKConditionListModel::overrideOwningForm(dovah::loaded_forms::Form& form) {
   this->_context = ui::types::conditions::context(form.stub, form.is_working_copy);
   if (this->_nodes.empty())
      return;
   auto tl = this->index(0, 0, {});
   auto br = this->index(this->_nodes.size() - 1, Column::_COUNT - 1, {});
   emit dataChanged(tl, br);
}

size_t DKConditionListModel::importBifurcatedList(dovah::loaded_forms::Form& src_form, const BackendConditionList& locked, const BackendConditionList& normal) {
   size_t invalid = 0;
   this->performReset([this, &src_form, &invalid, &locked, &normal]() {
      this->_allow_modifications_from = 0;

      auto _handle = [this, &invalid](const BackendConditionList& src, bool is_locked) {
         size_t size = src.size();
         this->_nodes.reserve(size);
         for (size_t i = 0; i < size; ++i) {
            auto* node = new node_type{ src[i] };
            if (!is_locked) {
               if (!node->valid()) {
                  ++invalid;
                  delete node;
                  continue;
               }
            }
            this->_nodes.push_back(node);
         }
      };
      _handle(locked, true);
      _handle(normal, false);

      this->_context = ui::types::conditions::context(src_form.stub, src_form.is_working_copy);
   });
   return invalid;
}
void DKConditionListModel::exportBifurcatedList(dovah::loaded_forms::Form& dst_form, BackendConditionList& locked, BackendConditionList& normal) {
   normal.clear(dst_form);
   for (size_t i = this->_allow_modifications_from; i < this->_nodes.size(); ++i) {
      auto& src_item = *this->_nodes[i];
      auto& dst_item = normal.emplace_back();
      dst_item.commit(dst_form, src_item);
   }
}