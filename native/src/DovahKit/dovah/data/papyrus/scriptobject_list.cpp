#include "./scriptobject_list.h"
#include "dovah/forms/components/papyrus/attachment_data.h"
#include "./property_value.h"

namespace {
   namespace raw {
      using namespace dovah::loaded_forms::components::papyrus;
   }

   void _copy_property_value(
      const raw::property_value& src,
      dovah::papyrus::property_value& dst
   ) {
      std::visit(
         [&dst](const auto& casted) {
            using value_type = std::decay_t<decltype(casted)>;
            if constexpr (std::is_same_v<value_type, raw::property_object_value>) {
               dst = casted;
            } else if constexpr (std::is_same_v<value_type, std::vector<raw::property_object_value>>) {
               dst = std::vector<dovah::papyrus::property_object_value>{};
               auto& dst_casted = std::get<std::vector<dovah::papyrus::property_object_value>>(dst);

               size_t size = casted.size();
               dst_casted.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_casted[i] = casted[i];
               }
            } else {
               dst = casted;
            }
         },
         src
      );
   }
   
   void _overwrite_property_value(
      dovah::loaded_forms::Form& target_form,
      const dovah::papyrus::property_value& src,
      raw::property_value& dst
   ) {
      std::visit(
         [&src, &dst, &target_form](const auto& casted) {
            using value_type = std::decay_t<decltype(casted)>;
            if constexpr (std::is_same_v<value_type, dovah::papyrus::property_object_value>) {
               dst = raw::property_object_value{};
               auto& dst_casted = std::get<raw::property_object_value>(dst);
                  
               dst_casted.alias_id = casted.alias_id;
               dst_casted.form.set(target_form, casted.form);
            } else if constexpr (std::is_same_v<value_type, std::vector<dovah::papyrus::property_object_value>>) {
               dst = std::vector<raw::property_object_value>{};
               auto& dst_casted = std::get<std::vector<raw::property_object_value>>(dst);

               size_t size = casted.size();
               dst_casted.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  dst_casted[i].alias_id = casted[i].alias_id;
                  dst_casted[i].form.set(target_form, casted[i].form);
               }
            } else {
               dst = casted;
            }
         },
         src
      );
   }
}

namespace dovah::papyrus {
   scriptobject_list::scriptobject_list(scriptobject_readwrite_passkey, const raw::attachment_data& target) {
      for (const auto& src_script : target.scripts) {
         if (src_script.status == raw::script_status::removed)
            continue;

         auto& dst_script = this->objects.emplace_back();
         dst_script.scriptname = src_script.name;
         dst_script.inheritance.present_on_target = true;

         for (const auto& src_prop : src_script.properties) {
            if (src_prop.status == raw::property_status::defined_only_on_base)
               continue;
            if (src_prop.status == raw::property_status::inherited_and_removed)
               continue;
            auto& dst_prop = dst_script._properties.emplace_back();
            dst_prop._name = src_prop.name;
            dst_prop.inheritance.present_on_target = true;
            _copy_property_value(src_prop.value, dst_prop.value);
         }
      }
   }
   scriptobject_list::scriptobject_list(scriptobject_readwrite_passkey, const raw::attachment_data& target, const raw::attachment_data& base) {
      for (auto& src_script : base.scripts) {
         auto& dst_script = this->objects.emplace_back();
         dst_script.scriptname = src_script.name;
         dst_script.inheritance.present_on_base = true;

         size_t size = src_script.properties.size();
         dst_script._properties.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_prop = src_script.properties[i];
            auto& dst_prop = dst_script._properties[i];
            dst_prop._name = src_prop.name;
            dst_prop.inheritance.present_on_base = true;
            _copy_property_value(src_prop.value, dst_prop.value);
         }
      }

      auto _get_or_insert = [this](const std::string& scriptname) -> auto& {
         if (auto* item = this->get_script_by_name(scriptname))
            return *item;
         auto& item = this->objects.emplace_back();
         item.scriptname = scriptname;
         return item;
      };

      for (auto& src_script : target.scripts) {
         if (src_script.status == raw::script_status::defined_on_base) {
            //
            // For this case, the CK seems to act the same as if the REFR VMAD contains no mention of the 
            // script at all. Actually removing the script requires both the "inherited" and "removed" 
            // status flags.
            //
            continue;
         }

         auto& dst_script = _get_or_insert(src_script.name);
         //
         if (src_script.status == raw::script_status::overrides_base) {
            dst_script.inheritance.present_on_target = true;
         }
         if (src_script.status == raw::script_status::removed) {
            dst_script.inheritance.removed_on_target = true;
            continue;
         }

         for (auto& src_prop : src_script.properties) {
            if (auto* dst_prop = dst_script._get_property_by_name(src_prop.name)) {
               if (src_prop.status == raw::property_status::defined_only_on_base) {
                  continue;
               }
               if (src_prop.status == raw::property_status::defined_locally) {
                  dst_prop->inheritance.present_on_target = true;
               }
               if (src_prop.status == raw::property_status::inherited_and_removed) {
                  dst_prop->inheritance.present_on_target = true;
                  dst_prop->inheritance.removed_on_target = true;
                  continue;
               }
               dst_prop->inheritance.present_on_target = true;
               _copy_property_value(src_prop.value, dst_prop->value);
               continue;
            }
            auto& dst_prop = dst_script._properties.emplace_back();
            dst_prop._name = src_prop.name;
            dst_prop.inheritance.present_on_target = true;
            _copy_property_value(src_prop.value, dst_prop.value);
         }
      }
   }

   void scriptobject_list::_overwrite(scriptobject_readwrite_passkey, loaded_forms::Form& target_form, raw::attachment_data& target) const {
      target.scripts.reserve(this->objects.size());
      for (auto& src_script : this->objects) {
         if (src_script.inheritance.removed_on_target) // sanity
            continue;
         auto& dst_script = target.scripts.emplace_back();
         dst_script.name = src_script.name();
         dst_script.properties.reserve(src_script._properties.size());
         dst_script.status = raw::script_status::defined_locally;

         for (auto& src_prop : src_script._properties) {
            if (src_prop.inheritance.removed_on_target) // sanity
               continue;
            auto& dst_prop = dst_script.properties.emplace_back();
            dst_prop.name   = src_prop.name();
            dst_prop.status = raw::property_status::defined_locally;
            _overwrite_property_value(target_form, src_prop.value, dst_prop.value);
         }
      }
   }
   void scriptobject_list::_overwrite(scriptobject_readwrite_passkey, loaded_forms::Form& target_form, raw::attachment_data& target, const raw::attachment_data& base) const {
      //
      // Confirmed in xEdit: a REFR only includes an entry for an attached script if it alters the 
      // script data in some way.
      //

      for (auto& src_script : this->objects) {
         if (!src_script.inheritance.present_on_target)
            continue;

         const auto* inherited_script = base.lookup_script(src_script.scriptname);

         if (src_script.inheritance.removed_on_target && !inherited_script)
            continue;

         auto& dst_script = target.scripts.emplace_back();
         dst_script.name = src_script.scriptname;
         if (inherited_script) {
            dst_script.status = raw::script_status::overrides_base;
            if (src_script.inheritance.removed_on_target) {
               dst_script.status = raw::script_status::removed;
               continue;
            }
         }

         for (auto& src_prop : src_script._properties) {
            if (!src_prop.inheritance.present_on_target) // can occur for properties grabbed from the compiled PEX
               continue;

            const raw::property* inherited_property = nullptr;
            if (inherited_script)
               inherited_property = inherited_script->lookup_property(src_prop.name());

            if (src_prop.inheritance.removed_on_target && !inherited_property)
               continue;

            auto& dst_prop = dst_script.properties.emplace_back();
            dst_prop.name   = src_prop.name();
            dst_prop.status = raw::property_status::defined_locally;
            if (inherited_property) {
               if (src_prop.inheritance.removed_on_target) {
                  dst_prop.status = raw::property_status::inherited_and_removed;
                  continue;
               }
            }
            _overwrite_property_value(target_form, src_prop.value, dst_prop.value);
         }

      }
   }
}