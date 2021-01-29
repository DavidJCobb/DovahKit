#include "stringify_condition_argument.h"
#include <array>
#include <QObject>
#include "../../dovah/forms/components/conditions/arg_types.h"
#include "../../dovah/forms/Quest.h"
#include "actor_value_index_to_name.h"
#include "form_type_name_to_string.h"

namespace {
   namespace _ci {
      using namespace dovah::loaded_forms::components::condition_info;
   }
}
namespace editor_helpers {
   extern QString stringify_condition_argument(
      bool& incomplete_information,
      const dovah::loaded_forms::components::condition& cnd,
      int   arg_index,
      const dovah::loaded_forms::Package* owning_package,
      const dovah::loaded_forms::Quest* owning_quest
   ) {
      incomplete_information = false;
      if (arg_index < 0 || arg_index > 1) {
         incomplete_information = true;
         return QString();
      }
      //
      auto* type  = cnd.get_argument_type(arg_index);
      auto  under = cnd.get_argument_underlying_type(arg_index);
      auto& value = cnd.parameters[arg_index];
      if (type == &_ci::arg_types::ActorValue) {
         QString out = actor_value_index_to_name(value.dword);
         if (!out.isEmpty())
            return out;
      }
      //
      switch (under) {
         case _ci::arg_underlying_type::aliasID:
            if (value.dword == -1)
               return QObject::tr("NONE", "condition argument (no alias)");
            if (owning_quest) {
               if (auto* alias = owning_quest->lookup_alias_by_id(value.dword)) {
                  return QString::fromUtf8(alias->name.c_str());
               }
            }
            incomplete_information = true;
            return QObject::tr("Alias ID #%1", "condition argument (alias ID with no identifiable owning quest)").arg(value.dword);
         case _ci::arg_underlying_type::character:
            return QString("%1").arg(QChar(value.dword & 0xFF));
         case _ci::arg_underlying_type::event:
            {
               QString function;
               QString member;
               switch (value.dword & 0xFFFF) {
                  case _ci::event_function::GetIsID:
                     function = "GetIsID";
                     break;
                  case _ci::event_function::GetItemValue:
                     function = "GetItemValue";
                     break;
                  case _ci::event_function::GetValue:
                     function = "GetValue";
                     break;
                  case _ci::event_function::HasKeyword:
                     function = "HasKeyword";
                     break;
                  case _ci::event_function::IsInList:
                     function = "IsInList";
                     break;
                  default:
                     function = "???";
                     break;
               }
               switch ((value.dword >> 0x10) & 0xFFFF) {
                  case _ci::event_member::created_object:
                     member = "Created Object";
                     break;
                  case _ci::event_member::form:
                     member = "Form";
                     break;
                  case _ci::event_member::keyword:
                     member = "Keyword";
                     break;
                  case _ci::event_member::location_new:
                     member = "Location (New)";
                     break;
                  case _ci::event_member::location_old:
                     member = "Location (Old)";
                     break;
                  case _ci::event_member::value_1:
                  case _ci::event_member::value_2:
                     member = "Int16";
                     break;
               }
               if (member.isEmpty())
                  return function;
               return QObject::tr("%1: %2").arg(function).arg(member);
            }
         case _ci::arg_underlying_type::float32:
            return QString("%1").arg(value.float32);
         case _ci::arg_underlying_type::int_signed:
            return QString("%1").arg((int32_t)value.dword);
         case _ci::arg_underlying_type::int_unsigned:
         case _ci::arg_underlying_type::quest_stage:
            return QString("%1").arg((uint32_t)value.dword);
         case _ci::arg_underlying_type::package_data:
            if (value.dword == -1)
               return QObject::tr("NONE", "condition argument (no package data)");
            if (owning_package) {
               //
               // TODO
               //
            }
            incomplete_information = true;
            return QObject::tr("Package Data #%1", "condition argument (package data index with no identifiable owning quest)").arg(value.dword);
         case _ci::arg_underlying_type::string:
            return QString::fromUtf8(value.string.c_str());
         case _ci::arg_underlying_type::formID:
            if (auto* stub = value.form.get_form_stub()) {
               if (!stub->is_none_stub()) {
                  auto tn = form_type_name_to_string(stub->formType);
                  auto id = stub->get_editor_id();
                  if (!tn.isEmpty())
                     return QObject::tr("%1: '%2'", "condition argument (form)").arg(tn).arg(id);
                  return QObject::tr("Form: '%1'", "condition argument (form of strange type)").arg(id);
               }
            }
            return QObject::tr("NONE", "condition argument (no form or none-stub)");
      }
      //
      incomplete_information = true;
      return QString();
   }
}