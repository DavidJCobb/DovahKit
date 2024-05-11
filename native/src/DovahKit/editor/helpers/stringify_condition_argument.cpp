#include "./stringify_condition_argument.h"
#include <array>
#include <QObject>
#include "dovah/data/conditions/all_parameter_types.h"
#include "dovah/data/conditions/event_function.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/components/conditions.h"
#include "dovah/forms/Quest.h"
#include "./actor_value_index_to_name.h"
#include "./form_type_name_to_string.h"

namespace editor_helpers {
   extern QString stringify_condition_argument(
      bool& incomplete_information,
      const dovah::loaded_forms::components::condition& cnd,
      int   arg_index,
      const dovah::loaded_forms::components::conditions::context& context
   ) {
      incomplete_information = false;
      if (arg_index < 0 || arg_index > 1) {
         incomplete_information = true;
         return QString();
      }
      //
      auto* func = cnd.get_function();
      if (func && func->uses_event_data) {
         const auto& params = cnd.get_event_parameters();
         switch (arg_index) {
            case 0:
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
            case 1:
               if (auto* q = context.get_owning_quest()) {
                  if (auto* e = dovah::story_event_definition::lookup(q->event))
                     if (auto* m = e->member_by_signature(params.member))
                        return m->name;
               }
               return QObject::tr("<event member:%1>").arg(params.member, 4, 16, QChar('0'));
            case 2:
               if (!dovah::conditions::event_function_uses_form(params.function))
                  return "";
               if (auto* stub = params.form.get_form_stub()) {
                  if (!stub->is_none_stub()) {
                     auto tn = form_type_name_to_string(stub->form_type);
                     auto id = stub->get_editor_id();
                     if (!tn.isEmpty())
                        return QObject::tr("%1: '%2'", "condition argument (form)").arg(tn).arg(id);
                     return QObject::tr("Form: '%1'", "condition argument (form of strange type)").arg(id);
                  }
               }
               return QObject::tr("NONE", "condition argument (no form or none-stub)");
         }
         return "";
      }
      //
      auto* type  = cnd.get_argument_type(arg_index);
      auto  under = cnd.get_argument_underlying_type(arg_index);
      auto& value = cnd.get_parameter(arg_index);

      if (std::holds_alternative<uint32_t>(value)) {
         auto dword = std::get<uint32_t>(value);
         if (type == &dovah::conditions::parameter_types::ActorValue) {
            QString out = actor_value_index_to_name(dword);
            if (!out.isEmpty())
               return out;
         }
         switch (under) {
            case dovah::conditions::parameter_underlying_type::alias:
               if (dword == -1)
                  return QObject::tr("NONE", "condition argument (no alias)");
               if (auto* q = context.get_owning_quest()) {
                  if (auto* alias = q->lookup_alias_by_id(dword)) {
                     return QString::fromUtf8(alias->name.c_str());
                  }
               }
               incomplete_information = true;
               return QObject::tr("Alias ID #%1", "condition argument (alias ID with no identifiable owning quest)").arg(dword);
            case dovah::conditions::parameter_underlying_type::int_unsigned:
            case dovah::conditions::parameter_underlying_type::quest_stage:
               return QString("%1").arg(dword);
            case dovah::conditions::parameter_underlying_type::package_data:
               if (dword == -1)
                  return QObject::tr("NONE", "condition argument (no package data)");
               if (context.package) {
                  //
                  // TODO
                  //
               }
               incomplete_information = true;
               return QObject::tr("Package Data #%1", "condition argument (package data index with no identifiable owning package)").arg(dword);
         }
         return QString::number(dword);
      }
      if (std::holds_alternative<char>(value)) {
         return QChar(std::get<char>(value));
      }
      if (std::holds_alternative<float>(value)) {
         return QString::number(std::get<float>(value));
      }
      if (std::holds_alternative<std::string>(value)) {
         return QString::fromStdString(std::get<std::string>(value));
      }
      if (std::holds_alternative<dovah::form_stub*>(value)) {
         auto* stub = std::get<dovah::form_stub*>(value);
         if (stub) {
            if (!stub->is_none_stub()) {
               auto tn = form_type_name_to_string(stub->form_type);
               auto id = stub->get_editor_id();
               if (!tn.isEmpty())
                  return QObject::tr("%1: '%2'", "condition argument (form)").arg(tn).arg(id);
               return QObject::tr("Form: '%1'", "condition argument (form of strange type)").arg(id);
            }
         }
         return QObject::tr("NONE", "condition argument (no form or none-stub)");
      }
      if (std::holds_alternative<int32_t>(value)) {
         auto integer = std::get<int32_t>(value);
         switch (under) {
            case dovah::conditions::parameter_underlying_type::enumeration:
               if (type->enumeration_info.has_value()) {
                  auto& enumeration = type->enumeration_info.value();
                  for (size_t i = 0; i < enumeration.size; ++i) {
                     const auto& m = enumeration.members[i];
                     if (integer == m.value)
                        return QString::fromUtf8(QByteArray(m.name.data(), m.name.size()));
                  }
               }
               break;
            case dovah::conditions::parameter_underlying_type::int_signed:
               return QString::number(integer);
         }
         return QString::number(integer);
      }
      if (std::holds_alternative<std::monostate>(value)) {
         return {};
      }
      return {};
   }
}