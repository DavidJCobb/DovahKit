#include "stringify_conditions.h"
#include "stringify_condition_argument.h"
#include "../core.h"
#include <QObject>
#include "../../dovah/data/story_manager.h"
#include "../../dovah/forms/Package.h"
#include "../../dovah/forms/Quest.h"

namespace {
   using condition = dovah::loaded_forms::components::condition;
}

namespace editor_helpers {
   extern QString stringify_condition(
      const dovah::loaded_forms::components::condition& cnd,
      const dovah::loaded_forms::components::condition_context& ctx
   ) {
      QString out;
      auto& editor = DovahKitCore::get();
      auto& rod    = cnd.get_run_on_data();
      //
      switch (rod.type) {
         case condition::run_on_type::subject:
            out += QObject::tr("Subject");
            break;
         case condition::run_on_type::target:
            out += QObject::tr("Target");
            break;
         case condition::run_on_type::reference:
            if (auto* stub = rod.reference.get_form_stub()) {
               if (!stub->is_none_stub()) {
                  auto* edid = stub->get_editor_id();
                  if (edid && edid[0])
                     out += edid;
                  else {
                     out += QString("[REFR:%1]").arg(stub->formID, 8, 16, QChar('0')).toUpper();
                  }
               } else {
                  out += QObject::tr("NONE", "condition - missing run-on ref");
               }
            } else {
               out += QObject::tr("NONE", "condition - missing run-on ref");
            }
            break;
         case condition::run_on_type::combat_target:
            out += QObject::tr("Combat Target");
            break;
         case condition::run_on_type::linked_ref:
            out += QObject::tr("Linked Ref");
            break;
         case condition::run_on_type::quest_alias:
            if (auto* q = ctx.get_owning_quest()) {
               auto* alias = q->lookup_alias_by_id(rod.index);
               if (alias)
                  out += alias->name.c_str();
               else
                  out += QObject::tr("Alias #%1").arg(rod.index);
            } else {
               out += QObject::tr("Alias #%1").arg(rod.index);
            }
            break;
         case condition::run_on_type::package_data:
            out += QObject::tr("Package Data #%1").arg(rod.index);
            break;
         case condition::run_on_type::event_data:
            if (auto* q = ctx.get_owning_quest()) {
               auto  code = q->event;
               auto* def  = dovah::story_event_definition::lookup(code);
               if (def) {
                  auto* member = def->member_by_wide_signature(rod.index);
                  if (member)
                     return member->name;
               }
            } else {
               out += QObject::tr("Event Data %1").arg(rod.index);
            }
            break;
         default:
            out += QObject::tr("?", "unknown condition run-on type enum");
            break;
      }
      out += QObject::tr(".", "condition run-on type delimiter");
      //
      auto* func = cnd.get_function();
      if (func)
         out += func->name;
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
      switch (cmp.op) {
         case condition::operator_type::equal:
            out += QObject::tr("==", "condition operator");
            break;
         case condition::operator_type::not_equal:
            out += QObject::tr("!=", "condition operator");
            break;
         case condition::operator_type::greater:
            out += QObject::tr(">", "condition operator");
            break;
         case condition::operator_type::greater_or_equal:
            out += QObject::tr(">=", "condition operator");
            break;
         case condition::operator_type::less:
            out += QObject::tr("<", "condition operator");
            break;
         case condition::operator_type::less_or_equal:
            out += QObject::tr("<=", "condition operator");
            break;
      }
      out += QObject::tr(" ", "condition operator padding");
      //
      if (cnd.get_flags() & condition::flag::compare_to_global) {
         auto* stub = cmp.operand.global.get_form_stub();
         if (stub && !stub->is_none_stub()) {
            out += stub->get_editor_id();
         } else {
            out += QObject::tr("NONE", "condition - missing global");
         }
      } else {
         out += QString::number(cmp.operand.constant);
      }
      return out;
   }

   extern QString stringify_condition_list(
      const dovah::loaded_forms::components::condition_list& list,
      const dovah::loaded_forms::components::condition_context& ctx
   ) {
      QString out;
      size_t  size = list.size();
      if (!size)
         return out;
      for (size_t i = 0; i < size - 1; ++i) {
         const auto& cnd = list[i];
         out += stringify_condition(cnd, ctx);
         if (cnd.get_flags() & condition::flag::or_linked)
            out += QObject::tr(" OR ");
         else
            out += QObject::tr(" AND ");
      }
      if (size)
         out += stringify_condition(list[size - 1], ctx);
      //
      return out;
   }
}