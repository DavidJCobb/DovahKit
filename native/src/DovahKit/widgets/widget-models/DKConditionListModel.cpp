#include "./DKConditionListModel.h"
#include <QColor>
#include <QFont>
#include "helpers/qt/strings.h"
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/all_parameter_types.h"
#include "dovah/data/conditions/event_function.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "editor/helpers/actor_value_index_to_name.h"
#include "editor/helpers/form_type_name_to_string.h"
#include "editor/core.h"

#include "dovah/forms/Package.h"
#include "dovah/forms/Quest.h"


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
}

DKConditionListModel::DKConditionListModel(QObject* parent) : DKGenericListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &DKConditionListModel::clear);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &DKConditionListModel::formModified);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DKConditionListModel::formDeletionImminent);
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

QString DKConditionListModel::_stringify_condition_parameter(const Condition& condition, size_t i) const {
   auto* function_info = dovah::conditions::function_info_by_id(condition.function);
   if (function_info == nullptr) {
      return {};
   }
   if (function_info->uses_event_data) {
      auto& params_opt = condition.event_parameters;
      if (!params_opt.has_value())
         return {};
      auto& params = params_opt.value();

      if (i == 0) {
         switch (params.function) {
            case dovah::conditions::event_function::GetIsID:
               return "GetIsID";
            case dovah::conditions::event_function::GetItemValue:
               return "GetItemValue";
            case dovah::conditions::event_function::GetValue:
               return "GetValue";
            case dovah::conditions::event_function::HasKeyword:
               return "HasKeyword";
            case dovah::conditions::event_function::IsInList:
               return "IsInList";
         }
         return QObject::tr("<event function:%1>").arg(params.function);
      } else if (i == 1) {
         if (auto* q = this->_context.get_owning_quest()) {
            if (auto* e = dovah::story_event_definition::lookup(q->event))
               if (auto* m = e->member_by_signature(params.member))
                  return m->name;
         }
         return QObject::tr("<event member:%1>").arg(params.member, 4, 16, QChar('0'));
      } else if (i == 2) {
         if (!dovah::conditions::event_function_uses_form(params.function))
            return {};
         if (const auto* stub = params.form) {
            if (!stub->is_none_stub()) {
               auto tn = editor_helpers::form_type_name_to_string(stub->form_type);
               auto id = stub->get_editor_id();
               if (!tn.isEmpty())
                  return QObject::tr("%1: '%2'", "condition argument (form)").arg(tn).arg(id);
               return QObject::tr("Form: '%1'", "condition argument (form of strange type)").arg(id);
            }
         }
         return QObject::tr("NONE", "condition argument (no form or none-stub)");
      }
      return {};
   }

   auto& parameter = condition.parameters[i];

   #pragma region Special-case functions
   if (condition.function == special_case_functions::IsSceneActionComplete) {
      //
      // TODO: First parameter is a Scene form; second parameter is the index of an action in that 
      //       scene. When we can load Scenes, show a drop-down of the actions instead of a spinbox.
      // 
      // TODO: Should we handle GetStageDone's quest stage parameter the same way, and remove the 
      //       "quest stage" type that's built into the condition internals?
      //
   }
   if (auto* casted = std::get_if<int32_t>(&parameter)) {
      auto value = *casted;
      if (condition.function == special_case_functions::IsLimbGone) {
         static constexpr const auto names = std::array{
            "Torso",
            "Head",
            "Eye",
            "Look At",
            "Fly Grab",
            "Saddle",
         };
         if (value < names.size()) {
            return tr("%1 (%2)", "IsLimbGone special-case names").arg(value).arg(names[value]);
         }
      } else if (condition.function == special_case_functions::IsPlayerActionActive) {
         static const auto names = std::array{
            tr("Swing Melee Weapon",    "PLAYER_ACTION"),
            tr("Cast Spell",            "PLAYER_ACTION"),
            tr("Shooting Bow",          "PLAYER_ACTION"),
            tr("Grabbing (Z-Key) Ref",  "PLAYER_ACTION"),
            tr("Knocking Over Objects", "PLAYER_ACTION"),
            tr("Standing on Furniture", "PLAYER_ACTION"),
            tr("Zoomed-In Aim",         "PLAYER_ACTION"),
            tr("Destroy Object",        "PLAYER_ACTION"),
            tr("Locked Object",         "PLAYER_ACTION"),
            tr("Pickpocket Crosshair",  "PLAYER_ACTION"),
            tr("Cast Self Spell",       "PLAYER_ACTION"),
            tr("Shout",                 "PLAYER_ACTION"),
            tr("Actor Collision",       "PLAYER_ACTION"),
         };
         if (value < names.size()) {
            return names[value];
         }
      }
   }
   #pragma endregion

   if (condition.get_argument_typeinfo(i) == &dovah::conditions::parameter_types::ActorValue) {
      if (auto* casted = std::get_if<uint32_t>(&parameter)) {
         QString out = editor_helpers::actor_value_index_to_name(*casted);
         if (!out.isEmpty())
            return out;
      }
      return tr("<MISMATCHED>", "condition argument with mismatched type");
   }

   if (auto* casted = std::get_if<float>(&parameter)) {
      return QString::number(*casted);
   } else if (auto* casted = std::get_if<int32_t>(&parameter)) {
      return QString::number(*casted);
   } else if (auto* casted = std::get_if<std::string>(&parameter)) {
      return QString::fromUtf8(QByteArray::fromStdString(*casted));
   } else if (auto* casted = std::get_if<char>(&parameter)) {
      return QChar(*casted);
   } else if (std::holds_alternative<dovah::form_stub*>(parameter)) {
      auto* stub = std::get<dovah::form_stub*>(parameter);
      if (stub && !stub->is_none_stub()) {
         auto tn = editor_helpers::form_type_name_to_string(stub->form_type);
         auto id = stub->get_editor_id();
         if (!tn.isEmpty())
            return QObject::tr("%1: '%2'", "condition argument (form)").arg(tn).arg(id);
         return QObject::tr("Form: '%1'", "condition argument (form of strange type)").arg(id);
      }
      return QObject::tr("NONE", "condition argument (no form or none-stub)");
   } else if (std::holds_alternative<uint32_t>(parameter)) {
      auto dword = std::get<uint32_t>(parameter);
      if (!function_info) {
         return QString::number(dword);
      }

      auto underlying = condition.get_argument_underlying_type(i);
      switch (underlying) {
         using enum dovah::conditions::parameter_underlying_type;
         case alias:
            if (auto* quest = this->_context.get_owning_quest()) {
               if (auto* alias = quest->lookup_alias_by_id(dword)) {
                  return QString::fromUtf8(QByteArray::fromStdString(alias->name));
               }
               return QObject::tr("Alias ID #%1", "condition argument (alias ID with no identifiable owning quest)").arg(dword);
            }
            return {};

         case package_data:
            if (dword == -1)
               return QObject::tr("NONE", "condition argument (no package data)");
            if (auto* package = this->_context.get_owning_package()) {
               //
               // TODO
               //
            }
            return QObject::tr("Package Data #%1", "condition argument (packge data with no identifiable owning quest)").arg(dword);

         case int_unsigned:
            return QString::number(dword);

         case quest_stage:
            return QString::number(dword);
      }
   }

   return {};
}

QVariant DKConditionListModel::data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
   const auto* function = dovah::conditions::function_info_by_id(node.function);

   switch (column) {
      case Column::Target:
         {
            using run_on_type = ui::types::conditions::run_on_type;

            const dovah::loaded_forms::Alias* run_on_alias     = nullptr;
            const dovah::loaded_forms::Quest* run_on_quest     = nullptr;
            const dovah::form_stub*           run_on_reference = nullptr;

            uint32_t run_on_alias_id = -1;
            QString  run_on_alias_name;

            if (node.run_on.type == run_on_type::reference) {
               if (std::holds_alternative<dovah::form_stub*>(node.run_on.entity))
                  run_on_reference = std::get<dovah::form_stub*>(node.run_on.entity);
            } else if (node.run_on.type == run_on_type::quest_alias) {
               if (run_on_quest = this->_context.get_owning_quest()) {
                  if (!std::holds_alternative<uint32_t>(node.run_on.entity))
                     break;
                  run_on_alias_id = std::get<uint32_t>(node.run_on.entity);
                  run_on_alias    = run_on_quest->lookup_alias_by_id(run_on_alias_id);
                  if (run_on_alias) {
                     run_on_alias_name = QString::fromStdString(run_on_alias->name).trimmed();
                  }
               }
            }

            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               switch (node.run_on.type) {
                  case run_on_type::combat_target:
                     return tr("Combat Target", "run on");
                  case run_on_type::event_data:
                     if (run_on_quest) {
                        if (std::holds_alternative<uint32_t>(node.run_on.entity)) {
                           auto  code = run_on_quest->event;
                           auto* def  = dovah::story_event_definition::lookup(code);
                           if (def) {
                              auto* member = def->member_by_wide_signature(std::get<uint32_t>(node.run_on.entity));
                              if (member)
                                 return tr("Event Data: %1", "run on").arg(member->name);
                           }
                        }
                     }
                     return tr("Event Data", "run on");
                  case run_on_type::linked_ref:
                     return tr("Linked Ref", "run on");
                  case run_on_type::package_data:
                     //
                     // TODO: check index; display which data
                     //
                     return tr("Package Data", "run on");
                  case run_on_type::quest_alias:
                     if (!run_on_alias_name.isEmpty())
                        return run_on_alias_name;
                     return tr("Alias ID #%1", "run on").arg(run_on_alias_id);
                  case run_on_type::reference:
                     if (run_on_reference) {
                        if (run_on_reference->formID == dovah::hardcoded_form_ids::PlayerRef)
                           return tr("Player", "run on form - player");
                        return tr("[%1:%2]%3", "run on form")
                           .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(run_on_reference->form_type).signature))
                           .arg(run_on_reference->formID, 8, 16, QChar('0'))
                           .arg(run_on_reference->get_editor_id());
                     }
                     return tr("No Reference", "run on");
                  case run_on_type::subject:
                     return tr("Subject", "run on");
                  case run_on_type::target:
                     return tr("Target", "run on");
               }
               break;
            } else if (role == Qt::FontRole) {
               QFont italics;
               italics.setItalic(true);

               switch (node.run_on.type) {
                  case run_on_type::package_data:
                     // TODO: revisit this when we actually know what package data indices *are*
                     return italics;
                  case run_on_type::quest_alias:
                     if (!run_on_alias_name.isEmpty())
                        return italics;
                     break;
                  case run_on_type::reference:
                     if (!run_on_reference)
                        break;
                     [[fallthrough]];
                  case run_on_type::event_data:
                  case run_on_type::linked_ref:
                  case run_on_type::combat_target:
                  case run_on_type::subject:
                  case run_on_type::target:
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
               if (!function)
                  break;
               if (function->argument_types[0] != &dovah::conditions::parameter_types::None) {
                  bool dummy;
                  auto value_a = _stringify_condition_parameter(node, 0);
                  if (function->argument_types[1] && function->argument_types[1] != &dovah::conditions::parameter_types::None) {
                     auto value_b = _stringify_condition_parameter(node, 1);
                     return tr("%1, %2").arg(value_a).arg(value_b);
                  }
                  return value_a;
               }
               break;
         }
         break;
      case Column::Operator:
         if (role == Qt::DisplayRole) {
            switch (node.comparison.op) {
               using enum ui::types::conditions::comparison_operator;
               case equal:
                  return tr("==", "comparison operator, equal");
               case greater:
                  return tr(">",  "comparison operator, greater");
               case greater_or_equal:
                  return tr(">=", "comparison operator, greater or equal");
               case less:
                  return tr("<",  "comparison operator, less");
               case less_or_equal:
                  return tr("<=", "comparison operator, less or equal");
               case not_equal:
                  return tr("!=", "comparison operator, not equal");
            }
         }
         break;
      case Column::Operand:
         {
            bool compare_to_global = std::holds_alternative<dovah::form_stub*>(node.comparison.operand);

            if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
               if (compare_to_global) {
                  auto* stub = std::get<dovah::form_stub*>(node.comparison.operand);
                  if (!stub)
                     return tr("NONE", "compare to global (missing)");
                  return tr("[%1:%2]%3", "compare to global")
                     .arg(cobb::qt::four_cc_to_string(dovah::form_type_info::lookup(stub->form_type).signature))
                     .arg(stub->formID, 8, 16, QChar('0'))
                     .arg(stub->get_editor_id());
               }
               assert(std::holds_alternative<float>(node.comparison.operand));
               return std::get<float>(node.comparison.operand);
            } else if (role == Qt::FontRole) {
               QFont italics;
               italics.setItalic(true);

               if (compare_to_global)
                  return italics;
            }
         }
         break;
      case Column::UsesOr:
         if (role == Qt::DisplayRole) {
            if (node.flags.or_linked)
               return tr("OR", "condition link, or");
            return tr("AND", "condition link, and");
         }
         break;
   }

   return {};
}
Qt::ItemFlags DKConditionListModel::flags_of(const node_type& node, size_t column) const {
   return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
}
      
void DKConditionListModel::clear() {
   DKGenericListModel::clear();
   this->_context = {};
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
      int diff   = bottom - top;
      this->beginInsertRows(dummy, bottom, bottom + diff);
      for (int i = bottom; i >= top; --i) {
         auto* clone = new node_type{ *this->_nodes[i] };
         this->_nodes.insert(this->_nodes.begin() + bottom + 1, clone);
      }
      this->endInsertRows();
   }
}
QModelIndex DKConditionListModel::insertAt(const Condition& data, size_t at) {
   if (at >= this->_nodes.size())
      at = this->_nodes.size();

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
void DKConditionListModel::remove(const QItemSelection& indices) {
   if (!indices.size())
      return;
   
   auto& list = this->_nodes;
   QModelIndex dummy;
   for (const QItemSelectionRange& range : indices) {
      int top    = range.top();
      int bottom = range.bottom();
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
   *this->_nodes[row] = src;

   auto tl = this->index(row, 0, {});
   auto br = this->index(row, column_count - 1, {});
   emit dataChanged(tl, br);
}

// Returns number of invalid conditions discarded.
size_t DKConditionListModel::importFrom(dovah::loaded_forms::Form& src_form, const BackendConditionList& src) {
   size_t invalid = 0;
   this->performReset([this, &src_form, &src, &invalid]() {
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