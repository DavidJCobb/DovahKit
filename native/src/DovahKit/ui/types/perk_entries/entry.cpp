#include "./entry.h"
#include "dovah/forms/components/papyrus/fragment_data/perk_fragment_data.h"
#include "dovah/forms/structs/perk_effect.h"
#include "dovah/forms/Perk.h"
#include "editor/subsystems/game_localized_strings/core.h"

namespace ui::types::perk_entries {
   /*static*/ std::vector<entry> entry::pull_list_from_backend(const dovah::loaded_forms::Perk& perk) {
      using src_type = dovah::loaded_forms::structs::perk_effect;

      auto&       gls      = dovahkit::subsystems::game_localized_strings::core::get();
      const auto& src_list = perk.effects;
      const auto* papyrus  = dynamic_cast<dovah::loaded_forms::components::papyrus::perk_fragment_data*>(perk.script_data.fragment_data);

      std::vector<entry> dst_list;
      size_t size = src_list.size();
      dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_item = src_list[i];
         auto& dst_item = dst_list[i];
         dst_item.priority = src_item.priority;
         dst_item.rank     = src_item.rank;
         switch (src_item.get_type()) {
            case src_type::type::ability:
               {
                  auto& src_data = std::get<src_type::data_types::ability>(src_item.data);
                  auto& dst_data = dst_item.data.emplace<ui::types::perk_entries::spell_entry>();
                  dst_data.spell = src_data.spell.get_form_stub();
               }
               break;
            case src_type::type::quest_and_stage:
               {
                  auto& src_data = std::get<src_type::data_types::quest>(src_item.data);
                  auto& dst_data = dst_item.data.emplace<ui::types::perk_entries::quest_entry>();
                  dst_data.quest = src_data.quest.get_form_stub();
                  dst_data.stage = src_data.stage;
               }
               break;
            case src_type::type::entry_point:
               {
                  auto& src_data = std::get<src_type::data_types::entry_point>(src_item.data);
                  auto& dst_data = dst_item.data.emplace<ui::types::perk_entries::entry_point_entry>();
                  dst_data.set_entry_point(src_data.entry);
                  dst_data.set_function(src_data.function.function);
                  switch (src_data.function.type()) {
                     using src_config_type = decltype(src_data.function);
                     case dovah::entry_point_function_type::activate_choice:
                        {
                           auto& src_params = std::get<src_config_type::data_types::activate_choice>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::activate_choice>();
                           dst_params.label = gls.convert_localized_string(src_params.text);
                           dst_params.spell = src_params.spell.get_form_stub();
                           dst_params.replace_default = src_params.flags & src_config_type::data_types::activate_choice::flag::replace_default;
                           dst_params.run_immediately = src_params.flags & src_config_type::data_types::activate_choice::flag::run_immediately;
                           if (papyrus) {
                              auto fi = src_params.fragment_index;
                              for (auto& frag : papyrus->fragments) {
                                 if (frag.index == fi) {
                                    dst_params.fragment.script   = QString::fromStdString(frag.filename);
                                    dst_params.fragment.function = QString::fromStdString(frag.function);
                                    break;
                                 }
                              }
                           }
                        }
                        break;
                     case dovah::entry_point_function_type::animation_graph_var:
                        {
                           auto& src_params = std::get<std::string>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::raw_string>();
                           dst_params.value = QString::fromStdString(src_params);
                        }
                        break;
                     case dovah::entry_point_function_type::leveled_item:
                        {
                           auto& src_params = std::get<src_config_type::data_types::leveled_item>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::form>();
                           dst_params.value = src_params.form.get_form_stub();
                        }
                        break;
                     case dovah::entry_point_function_type::localized_string:
                        {
                           auto& src_params = std::get<dovah::localized_string>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::localized_string>();
                           dst_params.value = gls.convert_localized_string(src_params);
                        }
                        break;
                     case dovah::entry_point_function_type::none:
                        dst_data.parameters.emplace<std::monostate>();
                        break;
                     case dovah::entry_point_function_type::one_float:
                        {
                           auto& src_params = std::get<src_config_type::data_types::one_float>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::one_float>();
                           dst_params.value = src_params.value;
                        }
                        break;
                     case dovah::entry_point_function_type::spell:
                        {
                           auto& src_params = std::get<src_config_type::data_types::spell>(src_data.function.data);
                           auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::form>();
                           dst_params.value = src_params.form.get_form_stub();
                        }
                        break;
                     case dovah::entry_point_function_type::two_floats:
                        {
                           auto& src_params = std::get<src_config_type::data_types::two_floats>(src_data.function.data);
                           if (dovah::entry_point_function_takes_an_av(dst_data.function)) {
                              auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::one_av_one_float>();
                              dst_params.actor_value = src_params.a;
                              dst_params.value       = src_params.b;
                           } else {
                              auto& dst_params = dst_data.parameters.emplace<ui::types::perk_entries::params::two_floats>();
                              dst_params.values[0] = src_params.a;
                              dst_params.values[1] = src_params.b;
                           }
                        }
                        break;
                  }
                  for (size_t i = 0; i < src_data.condition_groups.size(); ++i) {
                     auto& src_group = src_data.condition_groups[i];
                     auto  index     = src_group.which;
                     if (index >= dst_data.conditions_by_entity.size())
                        break;
                     auto& dst_group = dst_data.conditions_by_entity[index];
                     size_t size = src_group.conditions.size();
                     dst_group.conditions.clear();
                     dst_group.conditions.resize(size);
                     for (size_t i = 0; i < size; ++i) {
                        dst_group.conditions[i] = src_group.conditions[i];
                     }
                  }
               }
               break;
         }
      }
      return dst_list;
   }

   void entry::append_into_backend(dovah::loaded_forms::Perk& perk) const {
      using backend_type = dovah::loaded_forms::structs::perk_effect;

      auto& gls      = dovahkit::subsystems::game_localized_strings::core::get();
      auto& dst_list = perk.effects;

      auto& papyrus   = perk.script_data;
      auto* frag_info = papyrus.fragment_data;

      auto& dst_item = dst_list.emplace_back();
      dst_item.priority = this->priority;
      dst_item.rank     = this->rank;
      if (std::holds_alternative<quest_entry>(this->data)) {
         const auto& casted_src = std::get<quest_entry>(this->data);
         auto& casted_dst = dst_item.data.emplace<backend_type::data_types::quest>();
         casted_dst.quest.set(perk, casted_src.quest);
         casted_dst.stage = casted_src.stage;
      } else if (std::holds_alternative<spell_entry>(this->data)) {
         const auto& casted_src = std::get<spell_entry>(this->data);
         auto& casted_dst = dst_item.data.emplace<backend_type::data_types::ability>();
         casted_dst.spell.set(perk, casted_src.spell);
      } else if (std::holds_alternative<entry_point_entry>(this->data)) {
         const auto& casted_src = std::get<entry_point_entry>(this->data);
         auto& casted_dst = dst_item.data.emplace<backend_type::data_types::entry_point>();
         casted_dst.entry = casted_src.entry_point;
         casted_dst.function.function = casted_src.function;
         auto expected_param_type = dovah::expected_type_for_entry_point_function(casted_src.function);
         {
            using backend_type = dovah::loaded_forms::structs::perk_entry_point_data;

            auto& src_params = casted_src.parameters;
            auto& dst_params = casted_dst.function.data;
            if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::activate_choice>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<backend_type::data_types::activate_choice>();
               gls.assign_localized_string(casted_dst_params.text, casted_src_params->label);
               casted_dst_params.spell.set(perk, casted_src_params->spell);
               if (casted_src_params->replace_default)
                  casted_dst_params.flags |= backend_type::data_types::activate_choice::flag::replace_default;
               if (casted_src_params->run_immediately)
                  casted_dst_params.flags |= backend_type::data_types::activate_choice::flag::run_immediately;
               if (!casted_src_params->fragment.script.isEmpty() && !casted_src_params->fragment.function.isEmpty()) {
                  auto* perk_frag = dynamic_cast<dovah::loaded_forms::components::papyrus::perk_fragment_data*>(frag_info);
                  if (!frag_info) {
                     perk_frag = new dovah::loaded_forms::components::papyrus::perk_fragment_data;
                     papyrus.fragment_data = perk_frag;
                  }
                  if (perk_frag) {
                     auto& dst_frag = perk_frag->fragments.emplace_back();
                     dst_frag.index = perk_frag->fragments.size() - 1;
                     casted_dst_params.fragment_index = dst_frag.index;
                     dst_frag.filename = casted_src_params->fragment.script.toStdString();
                     dst_frag.function = casted_src_params->fragment.function.toStdString();
                  }
               }
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::form>(&src_params)) {
               if (expected_param_type == dovah::entry_point_function_type::leveled_item) {
                  auto& casted_dst_params = dst_params.emplace<backend_type::data_types::leveled_item>();
                  casted_dst_params.form.set(perk, casted_src_params->value);
               } else if (expected_param_type == dovah::entry_point_function_type::spell) {
                  auto& casted_dst_params = dst_params.emplace<backend_type::data_types::spell>();
                  casted_dst_params.form.set(perk, casted_src_params->value);
               } else {
                  assert(false && "unreachable!");
               }
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::localized_string>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<dovah::localized_string>();
               gls.assign_localized_string(casted_dst_params, casted_src_params->value);
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::one_av_one_float>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<backend_type::data_types::two_floats>();
               casted_dst_params.a = casted_src_params->actor_value;
               casted_dst_params.b = casted_src_params->value;
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::one_float>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<backend_type::data_types::one_float>();
               casted_dst_params.value = casted_src_params->value;
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::raw_string>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<std::string>();
               casted_dst_params = casted_src_params->value.toStdString();
            } else if (auto* casted_src_params = std::get_if<ui::types::perk_entries::params::two_floats>(&src_params)) {
               auto& casted_dst_params = dst_params.emplace<backend_type::data_types::two_floats>();
               casted_dst_params.a = casted_src_params->values[0];
               casted_dst_params.b = casted_src_params->values[1];
            }
         }
         //
         // Condition groups:
         //
         auto& src_groups = casted_src.conditions_by_entity;
         auto& dst_groups = casted_dst.condition_groups;
         for (size_t i = 0; i < src_groups.size(); ++i) {
            auto& src_group = src_groups[i];
            if (src_group.conditions.empty())
               continue;
            auto& dst_group = dst_groups.emplace_back();
            dst_group.which = i;
            dst_group.conditions.reserve(src_group.conditions.size());
            for (auto& cnd : src_group.conditions)
               dst_group.conditions.append(perk, cnd);
         }
      }
   }

   bool entry::sever_references_to(const dovah::form_stub& stub) {
      bool  changed = false;
      auto& variant = this->data;
      if (auto* casted_ptr = std::get_if<quest_entry>(&variant)) {
         if (casted_ptr->quest == &stub) {
            casted_ptr->quest = nullptr;
            changed = true;
         }
      } else if (auto* casted_ptr = std::get_if<spell_entry>(&variant)) {
         if (casted_ptr->spell == &stub) {
            casted_ptr->spell = nullptr;
            changed = true;
         }
      } else if (auto* casted_ptr = std::get_if<entry_point_entry>(&variant)) {
         auto& params = casted_ptr->parameters;
         if (auto* casted_params = std::get_if<ui::types::perk_entries::params::activate_choice>(&params)) {
            if (casted_params->spell == &stub) {
               casted_params->spell = nullptr;
               changed = true;
            }
         } else if (auto* casted_params = std::get_if<ui::types::perk_entries::params::form>(&params)) {
            if (casted_params->value == &stub) {
               casted_params->value = nullptr;
               changed = true;
            }
         }
         for (auto& cnd_list : casted_ptr->conditions_by_entity) {
            for (auto& cnd : cnd_list.conditions)
               if (cnd.sever_outbound_references_to(&stub))
                  changed = true;
         }
      }
      return false;
   }
}
