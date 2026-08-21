#include "./non_event_parameter.h"
#include <QCoreApplication>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/all_parameter_types.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/components/conditions.h"
#include "../actor_value_index_to_name.h"
#include "./alias.h"
#include "./form.h"
#include "./package_data.h"

namespace {
   constexpr const auto lookup_function_id_by_name(std::string_view name) {
      for (const auto& info : dovah::conditions::all_vanilla_function_info)
         if (info.name == name)
            return info.id;
      throw;
   }
}

namespace editor_helpers::condition_to_string {
   static QString special_case_integer_parameter(uint16_t function_id, int32_t param) {
      if (function_id == lookup_function_id_by_name("IsLimbGone")) {
         static const auto names = std::array{
            QCoreApplication::translate("IsLimbGone param", "Torso"),
            QCoreApplication::translate("IsLimbGone param", "Head"),
            QCoreApplication::translate("IsLimbGone param", "Eye"),
            QCoreApplication::translate("IsLimbGone param", "Look At"),
            QCoreApplication::translate("IsLimbGone param", "Fly Grab"),
            QCoreApplication::translate("IsLimbGone param", "Saddle"),
         };
         if (param < names.size()) {
            return names[param];
         }
      } else if (function_id == lookup_function_id_by_name("IsPlayerActionActive")) {
         static const auto names = std::array{
            QCoreApplication::translate("PLAYER_ACTION", "Swing Melee Weapon"),
            QCoreApplication::translate("PLAYER_ACTION", "Cast Spell"),
            QCoreApplication::translate("PLAYER_ACTION", "Shooting Bow"),
            QCoreApplication::translate("PLAYER_ACTION", "Grabbing (Z-Key) Ref"),
            QCoreApplication::translate("PLAYER_ACTION", "Knocking Over Objects"),
            QCoreApplication::translate("PLAYER_ACTION", "Standing on Furniture"),
            QCoreApplication::translate("PLAYER_ACTION", "Zoomed-In Aim"),
            QCoreApplication::translate("PLAYER_ACTION", "Destroy Object"),
            QCoreApplication::translate("PLAYER_ACTION", "Locked Object"),
            QCoreApplication::translate("PLAYER_ACTION", "Pickpocket Crosshair"),
            QCoreApplication::translate("PLAYER_ACTION", "Cast Self Spell"),
            QCoreApplication::translate("PLAYER_ACTION", "Shout"),
            QCoreApplication::translate("PLAYER_ACTION", "Actor Collision"),
         };
         if (param < names.size()) {
            return names[param];
         }
      }
      return {};
   }
   
   extern QString non_event_parameter(
      const    dovah::loaded_forms::components::conditions::context& context,
      uint16_t function_id,
      const    dovah::conditions::parameter_typeinfo* argument_typeinfo,
      dovah::conditions::parameter_underlying_type    underlying_type,
      const    dovah::loaded_forms::components::conditions::working_parameter& parameter,
      size_t   which,
      bool     show_special_cases,
      const    options::form_format& form_format
   ) {
      auto* function_info = dovah::conditions::function_info_by_id(function_id);
      if (!function_info)
         return {};

      if (which == 0) {
         if (auto* casted = std::get_if<int32_t>(&parameter)) {
            auto special_case = special_case_integer_parameter(function_id, *casted);
            if (!special_case.isEmpty())
               return special_case;
         }
      }

      if (argument_typeinfo == &dovah::conditions::parameter_types::ActorValue) {
         if (auto* casted = std::get_if<int32_t>(&parameter)) {
            QString out = actor_value_index_to_name(*casted);
            if (!out.isEmpty())
               return out;
         }
         return QCoreApplication::translate("condition parameter - invalid", "???");
      }
      
      if (auto* casted = std::get_if<float>(&parameter)) {
         return QString::number(*casted);
      } else if (auto* casted = std::get_if<int32_t>(&parameter)) {
         if (argument_typeinfo && argument_typeinfo->enumeration_info.has_value()) {
            //
            // NOTE: If we want to localize enumeration members' names, we could do that HERE if we 
            // check what enum we're working with (i.e. identity comparison on *argument_typeinfo).
            //
            const auto& info = argument_typeinfo->enumeration_info.value();
            for (size_t i = 0; i < info.size; ++i) {
               const auto& member = info.members[i];
               if (*casted == member.value) {
                  const std::string_view& name = member.name;
                  return QString::fromLatin1(QByteArray(name.data(), name.size()));
               }
            }
         }
         return QString::number(*casted);
      } else if (auto* casted = std::get_if<std::string>(&parameter)) {
         return QString::fromUtf8(QByteArray::fromStdString(*casted));
      } else if (auto* casted = std::get_if<char>(&parameter)) {
         return QChar(*casted);
      } else if (std::holds_alternative<dovah::form_stub*>(parameter)) {
         auto* stub = std::get<dovah::form_stub*>(parameter);
         return form(stub, form_format);
      } else if (std::holds_alternative<uint32_t>(parameter)) {
         auto dword = std::get<uint32_t>(parameter);
         switch (underlying_type) {
            using enum dovah::conditions::parameter_underlying_type;
            case alias:
               return condition_to_string::alias(context, dword, true).first;

            case package_data:
               return condition_to_string::package_data(context, dword).first;

            case int_unsigned:
               return QString::number(dword);

            case quest_stage:
               return QString::number(dword);
         }
         return QString::number(dword);
      }

      return {};
   }

   extern QString non_event_parameter(
      const  dovah::loaded_forms::components::conditions::context& context,
      const  dovah::loaded_forms::components::conditions::working_condition& cnd,
      size_t which,
      bool   show_special_cases,
      const  options::form_format& form_format
   ) {
      if (which >= 2)
         return {};
      return non_event_parameter(
         context,
         cnd.function,
         cnd.get_argument_typeinfo(which),
         cnd.get_argument_underlying_type(which),
         cnd.parameters[which],
         which,
         show_special_cases,
         form_format
      );
   }

   extern QString non_event_parameter(
      const  dovah::loaded_forms::components::conditions::context& context,
      const  dovah::loaded_forms::components::condition& cnd,
      size_t which,
      bool   show_special_cases,
      const  options::form_format& form_format
   ) {
      if (which >= 2)
         return {};
      return non_event_parameter(
         context,
         cnd.get_function_id(),
         cnd.get_argument_type(which),
         cnd.get_argument_underlying_type(which),
         cnd.get_parameter(which),
         which,
         show_special_cases,
         form_format
      );
   }
}
