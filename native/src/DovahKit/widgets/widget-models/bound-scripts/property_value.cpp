#include "./property_value.h"
#include "helpers/type_traits/is_std_vector.h"

#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"

// for stringifying quest aliases:
#include "dovah/forms/Quest.h"
#include "editor/core.h"

namespace ui::bound_script_models {
   void property_value_to_vmad(const property_value& src, vmad::property_value& dst, dovah::loaded_forms::Form& dst_form) {
      std::visit(
         [&dst, &dst_form](auto& src_v) {
            using value_type = std::decay_t<decltype(src_v)>;

            if constexpr (cobb::is_std_vector<value_type>) {
               using item_type = typename value_type::value_type;

               size_t size = src_v.size();
               if constexpr (std::is_same_v<item_type, QString>) {
                  auto& dst_v = dst.emplace<std::vector<std::string>>();
                  dst_v.resize(size);
                  for (size_t i = 0; i < size; ++i)
                     dst_v[i] = src_v[i].toUtf8().toStdString();
               } else if constexpr (std::is_same_v<item_type, ui::types::quest_alias>) {
                  auto& dst_v = dst.emplace<std::vector<vmad::property_object_value>>();
                  dst_v.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     if (src_v[i].empty())
                        continue;
                     dst_v[i].form.set(dst_form, src_v[i].quest);
                     dst_v[i].alias_id = src_v[i].alias_id;
                  }
               } else if constexpr (std::is_same_v<item_type, dovah::form_stub*>) {
                  auto& dst_v = dst.emplace<std::vector<vmad::property_object_value>>();
                  dst_v.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     dst_v[i].form.set(dst_form, src_v[i]);
                  }
               } else {
                  dst = src_v;
               }
            } else {
               if constexpr (std::is_same_v<value_type, QString>) {
                  dst = src_v.toUtf8().toStdString();
               } else if constexpr (std::is_same_v<value_type, ui::types::quest_alias>) {
                  auto& dst_v = dst.emplace<vmad::property_object_value>();
                  if (!src_v.empty()) {
                     dst_v.form.set(dst_form, src_v.quest);
                     dst_v.alias_id = src_v.alias_id;
                  }
               } else if constexpr (std::is_same_v<value_type, dovah::form_stub*>) {
                  auto& dst_v = dst.emplace<vmad::property_object_value>();
                  dst_v.form.set(dst_form, src_v);
               } else {
                  dst = src_v;
               }
            }
         },
         src
      );
   }

   namespace {
      QString _stringify(bool v) {
         return v ? "True" : "False";
      }
      QString _stringify(float v) {
         return QString::number(v);
      }
      QString _stringify(int32_t v) {
         return QString::number(v);
      }
      QString _stringify(const dovah::form_stub* v) {
         if (!v) {
            return "None";
         }
         auto form_str = editor_helpers::form_identifiers_to_string(v);
         if (!v->editorID.empty()) {
            return form_str;
         }

         if (auto* base = dovah::form_stub_helpers::get_base_form(v)) {
            if (!base->editorID.empty()) {
               return QString("%1 (%2)").arg(form_str).arg(QString::fromStdString(base->editorID));
            }
         }

         return form_str;
      }
      QString _stringify(const ui::types::quest_alias& v) {
         if (v.empty())
            return "None";

         QString alias_string;
         QString quest_string;

         if (v.quest->editorID.empty()) {
            quest_string = editor_helpers::form_identifiers_to_string(v.quest);
         } else {
            quest_string = QString::fromStdString(v.quest->editorID);
         }

         auto loaded_quest = v.quest->load().ptr_cast<dovah::loaded_forms::Quest>();
         if (loaded_quest) {
            if (auto* alias = loaded_quest->lookup_alias_by_id(v.alias_id)) {
               alias_string = QString::fromStdString(alias->name);
            }
         }
         if (alias_string.isEmpty()) {
            alias_string = QString("ID#%1").arg(v.alias_id);
         }

         return QString("%1 on %2").arg(alias_string).arg(quest_string);
      }
      QString _stringify(QString v) {
         return v;
      }
   }
   QString stringify(const property_value& src) {
      if (std::holds_alternative<std::monostate>(src)) {
         return "None";
      }
      QString out;
      std::visit(
         [&out](const auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               out = "[ ";
               size_t size = v.size();
               for (size_t i = 0; i < size; ++i) {
                  out += _stringify(v[i]);
                  if (i + 1 < size)
                     out += ", ";
               }
               out += " ]";
            } else if constexpr (!std::is_same_v<value_type, std::monostate>) {
               out = _stringify(v);
            }
         },
         src
      );
      return out;
   }
}