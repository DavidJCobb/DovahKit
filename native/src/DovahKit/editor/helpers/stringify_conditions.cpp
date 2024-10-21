#include "./stringify_conditions.h"
#include "./stringify_condition_argument.h"
#include "../core.h"
#include <QObject>
#include <QStringBuilder>
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/data/conditions/comparison_operator.h"
#include "dovah/data/conditions/function_info.h"
#include "dovah/data/conditions/run_on_type.h"
#include "dovah/data/story_manager.h"
#include "dovah/forms/components/conditions/working_condition.h"
#include "dovah/forms/Package.h"
#include "dovah/forms/Quest.h"
#include "dovah/form_stub.h"

namespace {
   namespace conditions {
      using namespace dovah::conditions;
      using namespace dovah::loaded_forms::components::conditions;
   }
   using condition = dovah::loaded_forms::components::condition;
   using context   = dovah::loaded_forms::components::conditions::context;
}

namespace {
   QString _stringify_comparison_operator(dovah::conditions::comparison_operator op) {
      switch (op) {
         case conditions::comparison_operator::equal:
            return QObject::tr("==", "condition operator");
         case conditions::comparison_operator::not_equal:
            return QObject::tr("!=", "condition operator");
         case conditions::comparison_operator::greater:
            return QObject::tr(">", "condition operator");
         case conditions::comparison_operator::greater_or_equal:
            return QObject::tr(">=", "condition operator");
         case conditions::comparison_operator::less:
            return QObject::tr("<", "condition operator");
         case conditions::comparison_operator::less_or_equal:
            return QObject::tr("<=", "condition operator");
      }
      std::unreachable();
      return QObject::tr("??", "condition operator");
   }

   QString _stringify_condition_run_on(const context& context, dovah::conditions::run_on_type type, uint32_t integral, dovah::form_stub* stub) {
      switch (type) {
         case conditions::run_on_type::subject:
            return QObject::tr("Subject");
         case conditions::run_on_type::target:
            return QObject::tr("Target");
         case conditions::run_on_type::reference:
            if (stub && !stub->is_none_stub()) {
               auto* edid = stub->get_editor_id();
               if (edid && edid[0])
                  return edid;
               else {
                  return QString("[REFR:%1]").arg(stub->formID, 8, 16, QChar('0')).toUpper();
               }
            } else {
               return QObject::tr("NONE", "condition - missing run-on ref");
            }
            break;
         case conditions::run_on_type::combat_target:
            return QObject::tr("Combat Target");
         case conditions::run_on_type::linked_ref:
            return QObject::tr("Linked Ref");
         case conditions::run_on_type::quest_alias:
            if (auto* q = context.get_owning_quest()) {
               auto* alias = q->lookup_alias_by_id(integral);
               if (alias)
                  return alias->name.c_str();
               else
                  return QObject::tr("Alias #%1").arg(integral);
            }
            return QObject::tr("Alias #%1").arg(integral);
         case conditions::run_on_type::package_data:
            return QObject::tr("Package Data #%1").arg(integral);
            break;
         case conditions::run_on_type::event_data:
            if (auto* q = context.get_owning_quest()) {
               auto  code = q->event;
               auto* def  = dovah::story_event_definition::lookup(code);
               if (def) {
                  auto* member = def->member_by_wide_signature(integral);
                  if (member)
                     return member->name;
               }
            }
            return QObject::tr("Event Data %1").arg(integral);
      }
      return QObject::tr("?", "unknown condition run-on type enum");
   }
}

namespace editor_helpers {
   extern QString stringify_condition(
      const dovah::loaded_forms::components::condition& cnd,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      auto& editor = DovahKitCore::get();
      auto& rod    = cnd.get_run_on_data();
      //
      out += _stringify_condition_run_on(ctx, rod.type, rod.index, rod.reference.get_form_stub());
      out += QObject::tr(".", "condition run-on type delimiter");
      //
      auto* func = cnd.get_function();
      if (func)
         out += QString::fromUtf8(QByteArray(func->name.data(), func->name.size()));
      else
         out += QObject::tr("?%1", "unknown condition function").arg(cnd.get_function_id());
      out += QObject::tr("(", "condition arg delimiter - open");
      if (func) {
         bool dummy;
         auto count = func->argument_count();
         for (int i = 0; i < count; ++i) {
            out += stringify_condition_argument(dummy, cnd, i, ctx);
            if (i + 1 < count)
               out += QObject::tr(",", "condition arg delimiter - separator");
         }
      }
      out += QObject::tr(")", "condition arg delimiter - close");
      //
      auto& cmp = cnd.get_comparison();
      out += QObject::tr(" ", "condition operator padding");
      out += _stringify_comparison_operator(cmp.op);
      out += QObject::tr(" ", "condition operator padding");
      //
      if (std::holds_alternative<dovah::form_reference_t>(cmp.operand)) {
         const auto* stub = std::get<dovah::form_reference_t>(cmp.operand).get_form_stub();
         if (stub && !stub->is_none_stub()) {
            out += stub->get_editor_id();
         } else {
            out += QObject::tr("NONE", "condition - missing global");
         }
      } else {
         out += QString::number(std::get<float>(cmp.operand));
      }
      return out;
   }
   extern QString stringify_condition(
      const dovah::loaded_forms::components::conditions::working_condition& cnd,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      auto& editor = DovahKitCore::get();

      {
         uint32_t          index = 0;
         dovah::form_stub* stub  = nullptr;
         if (std::holds_alternative<dovah::form_stub*>(cnd.run_on.entity)) {
            stub = std::get<dovah::form_stub*>(cnd.run_on.entity);
         } else if (std::holds_alternative<uint32_t>(cnd.run_on.entity)) {
            index = std::get<uint32_t>(cnd.run_on.entity);
         }
         out += _stringify_condition_run_on(ctx, cnd.run_on.type, index, stub);
      }
      out += QObject::tr(".", "condition run-on type delimiter");

      const auto* func = dovah::conditions::function_info_by_id(cnd.function);
      if (func)
         out += QString::fromUtf8(QByteArray(func->name.data(), func->name.size()));
      else
         out += QObject::tr("?%1", "unknown condition function").arg(cnd.function);
      out += QObject::tr("(", "condition arg delimiter - open");
      if (func) {
         bool dummy;
         auto count = func->argument_count();
         for (int i = 0; i < count; ++i) {
            out += stringify_condition_argument(dummy, cnd, i, ctx);
            if (i + 1 < count)
               out += QObject::tr(",", "condition arg delimiter - separator");
         }
      }
      out += QObject::tr(")", "condition arg delimiter - close");
      //
      out += QObject::tr(" ", "condition operator padding");
      out += _stringify_comparison_operator(cnd.comparison.op);
      out += QObject::tr(" ", "condition operator padding");
      if (std::holds_alternative<dovah::form_stub*>(cnd.comparison.operand)) {
         const auto* stub = std::get<dovah::form_stub*>(cnd.comparison.operand);
         if (stub && !stub->is_none_stub()) {
            out += stub->get_editor_id();
         } else {
            out += QObject::tr("NONE", "condition - missing global");
         }
      } else {
         out += QString::number(std::get<float>(cnd.comparison.operand));
      }
      return out;
   }

   extern QString stringify_condition_boolean_operator(
      const dovah::loaded_forms::components::condition& cnd
   ) {
      if (cnd.get_flags() & condition::flag::or_linked)
         return QObject::tr("OR", "condition boolean operator");
      else
         return QObject::tr("AND", "condition boolean operator");
   }
   extern QString stringify_condition_boolean_operator(
      const dovah::loaded_forms::components::conditions::working_condition& cnd
   ) {
      if (cnd.flags.or_linked)
         return QObject::tr("OR", "condition boolean operator");
      else
         return QObject::tr("AND", "condition boolean operator");
   }

   extern QString stringify_condition_list(
      const dovah::loaded_forms::components::condition_list& list,
      const dovah::loaded_forms::components::conditions::context& ctx
   ) {
      QString out;
      size_t  size = list.size();
      if (!size)
         return out;
      for (size_t i = 0; i < size - 1; ++i) {
         const auto& cnd = list[i];
         out += stringify_condition(cnd, ctx);
         out += " " % stringify_condition_boolean_operator(cnd) % " ";
      }
      if (size)
         out += stringify_condition(list[size - 1], ctx);
      //
      return out;
   }
}