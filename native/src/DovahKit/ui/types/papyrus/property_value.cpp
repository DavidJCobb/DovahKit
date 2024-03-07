#include "./property_value.h"
#include "helpers/vectors/map_to_new_type.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"

namespace {
   using vmad_object_value = dovah::loaded_forms::components::papyrus::property_object_value;
}

namespace ui::types::papyrus {
   property_value vmad_property_value_to_ui(const dovah::loaded_forms::components::papyrus::property_value& src, std::optional<dovah::form_type> underlying_native_type) {
      if (underlying_native_type.has_value()) {

         //
         // Valid destination type is a native object.
         //

         bool is_alias = false;
         switch (underlying_native_type.value()) {
            case dovah::form_type::alias:
            case dovah::form_type::location_alias:
            case dovah::form_type::reference_alias:
               is_alias = true;
         }

         property_value dst;
         if (is_alias) {
            if (std::holds_alternative<vmad_object_value>(src)) {
               const auto& src_v = std::get<vmad_object_value>(src);

               auto* quest = src_v.form.get_form_stub();
               if (!quest || quest->form_type != dovah::form_type::quest) {
                  dst = ui::types::quest_alias{};
               } else {
                  dst = ui::types::quest_alias{
                     .quest    = quest,
                     .alias_id = src_v.alias_id,
                  };
               }
            } else if (std::holds_alternative<std::vector<vmad_object_value>>(src)) {
               dst = std::vector<ui::types::quest_alias>{};

               const auto& src_v = std::get<std::vector<vmad_object_value>>(src);
               auto&       dst_v = std::get<std::vector<ui::types::quest_alias>>(dst);

               size_t size = src_v.size();
               dst_v.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  auto* quest = src_v[i].form.get_form_stub();
                  if (!quest || quest->form_type != dovah::form_type::quest) {
                     dst = ui::types::quest_alias{};
                  } else {
                     dst = ui::types::quest_alias{
                        .quest    = quest,
                        .alias_id = src_v[i].alias_id,
                     };
                  }
               }
            }
            return dst;
         }

         if (underlying_native_type.value() == dovah::form_type::active_magic_effect) {
            return dst;
         }
         
         if (std::holds_alternative<vmad_object_value>(src)) {
            const auto& src_v = std::get<vmad_object_value>(src);

            if (src_v.alias_id != vmad_object_value::no_alias) {
               dst = (dovah::form_stub*)nullptr;
            } else {
               //
               // TODO: Validate the form type. Not as simple as comparing it to `underlying_native_type`, 
               //       because some form types actually inherit from one another (e.g. FURN subclasses ACTI) 
               //       and this is reflected and respected within Papyrus's binding rules.
               //
               dst = src_v.form.get_form_stub();
            }
         } else if (std::holds_alternative<std::vector<vmad_object_value>>(src)) {
            dst = std::vector<dovah::form_stub*>{};

            const auto& src_v = std::get<std::vector<vmad_object_value>>(src);
            auto&       dst_v = std::get<std::vector<dovah::form_stub*>>(dst);

            size_t size = src_v.size();
            dst_v.resize(size);
            for (size_t i = 0; i < size; ++i) {
               if (src_v[i].alias_id != vmad_object_value::no_alias) {
                  dst_v[i] = nullptr;
               } else {
                  //
                  // TODO: Validate the form type. Not as simple as comparing it to `underlying_native_type`, 
                  //       because some form types actually inherit from one another (e.g. FURN subclasses ACTI) 
                  //       and this is reflected and respected within Papyrus's binding rules.
                  //
                  dst_v[i] = src_v[i].form.get_form_stub();
               }
            }
         }

         //
         // Source type is not a native object. Return an empty property vlaue.
         //

         return dst;
      }

      //
      // Valid destination type is not a native object.
      // 

      property_value out;
      std::visit(
         [&out](const auto& src_v) {
            using src_value_type = std::decay_t<decltype(src_v)>;

            if constexpr (std::is_same_v<src_value_type, vmad_object_value> || std::is_same_v<src_value_type, std::vector<vmad_object_value>>) {
               //
               // Again, valid destination type is not a native object.
               //
               return;
            } else if constexpr (cobb::is_std_vector<src_value_type>) {
               //
               // Array types.
               //
               using src_element_type = typename src_value_type::value_type;

               if constexpr (std::is_same_v<src_element_type, std::string>) {
                  out = cobb::vectors::map_to_new_type<QString>(src_v, [](const auto& src, auto& dst) {
                     dst = QString::fromStdString(src);
                  });
               } else {
                  out = src_v;
               }
            } else {
               //
               // Non-array types.
               //
               if constexpr (std::is_same_v<src_value_type, std::string>) {
                  out = QString::fromStdString(src_v);
               } else {
                  out = src_v;
               }
            }
         },
         src
      );
      return out;
   }

   void ui_property_value_to_vmad(const property_value& src, dovah::loaded_forms::components::papyrus::property_value& dst, dovah::loaded_forms::Form& dst_owner) {
      //
      // If `dst` holds any `form_reference_t`s, then we need to clear those.
      //
      if (std::holds_alternative<vmad_object_value>(dst)) {
         std::get<vmad_object_value>(dst).form.set(dst_owner, nullptr);
      } else if (std::holds_alternative<std::vector<vmad_object_value>>(dst)) {
         auto& dst_v = std::get<std::vector<vmad_object_value>>(dst);
         for(auto& item : dst_v)
            item.form.set(dst_owner, nullptr);
         dst_v.clear();
      }
      dst = {};

      std::visit(
         [&dst, &dst_owner](const auto& src_v) {
            using src_value_type = std::decay_t<decltype(src_v)>;

            if constexpr (cobb::is_std_vector<src_value_type>) {
               using src_element_type = typename src_value_type::value_type;

               if constexpr (std::is_same_v<src_element_type, QString>) {
                  dst = cobb::vectors::map_to_new_type<std::string>(src_v, [](const auto& src, auto& dst) {
                     dst = src.toStdString();
                  });
               } else if constexpr (std::is_same_v< src_element_type, ui::types::quest_alias>) {
                  dst = cobb::vectors::map_to_new_type<vmad_object_value>(src_v, [&dst_owner](const auto& src, auto& dst) {
                     dst.form.set(dst_owner, src.quest);
                     dst.alias_id = src.alias_id;
                  });
               } else if constexpr (std::is_same_v<src_element_type, dovah::form_stub*>) {
                  dst = cobb::vectors::map_to_new_type<vmad_object_value>(src_v, [&dst_owner](const auto& src, auto& dst) {
                     dst.form.set(dst_owner, src);
                  });
               } else {
                  dst = src_v;
               }
            } else {
               if constexpr (std::is_same_v<src_value_type, QString>) {
                  dst = src_v.toStdString();
               } else if constexpr (std::is_same_v<src_value_type, ui::types::quest_alias>) {
                  dst = vmad_object_value{};
                  auto& dst_v = std::get<vmad_object_value>(dst);
                  dst_v.form.set(dst_owner, src_v.quest);
                  dst_v.alias_id = src_v.alias_id;
               } else if constexpr (std::is_same_v<src_value_type, dovah::form_stub*>) {
                  dst = vmad_object_value{};
                  auto& dst_v = std::get<vmad_object_value>(dst);
                  dst_v.form.set(dst_owner, src_v);
               } else {
                  dst = src_v;
               }
            }
         },
         src
      );


   }
}