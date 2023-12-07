#include "form.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../files/tes_file_reading/elements.h"
#include "../files/tes_file_writing/elements.h"
#include "factories/construct.h"
#include "_component_access.h"

#include "../form_stub_helpers.h"
#include "./components/papyrus/attachment_data.h"
#include "./components/papyrus/attached_script.h"
#include "./components/papyrus/property.h"

namespace dovah::loaded_forms {
   Form::Form(form_type_t ft, const constructor_params& c) : formType(ft), is_working_copy(c.is_working_copy), stub(*c.stub) {
      assert(c.stub && "Form::constructor_params::stub must not be nullptr at the time construction occurs!");
   }

   std::vector<dovah::papyrus::scriptobject> Form::resolve_papyrus_scripts() const {
      std::vector<dovah::papyrus::scriptobject> dst;

      loaded_form_ptr<Form> base_form;
      if (dovah::form_type_info::form_type_is_reference(this->stub.formType)) {
         auto* base_stub = form_stub_helpers::get_base_form(&this->stub);
         if (base_stub)
            base_form = base_stub->load();
      }

      auto _copy_papyrus_property_value = [](
         const components::papyrus::property_value& src,
         dovah::papyrus::property_value& dst
      ) {
         std::visit(
            [&dst](const auto& casted) {
               using value_type = std::decay_t<decltype(casted)>;
               if constexpr (std::is_same_v<value_type, components::papyrus::property_object_value>) {
                  dst = casted;
               } else if constexpr (std::is_same_v<value_type, std::vector<components::papyrus::property_object_value>>) {
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
      };

      if (base_form) {
         if (auto* base_papyrus = base_form->get_raw_papyrus_data()) {
            for (auto& src_script : base_papyrus->scripts) {
               auto& dst_script = dst.emplace_back();
               dst_script.scriptname = src_script.name;
               dst_script.inheritance.present_on_base = true;

               size_t size = src_script.properties.size();
               dst_script.properties.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  auto& src_prop = src_script.properties[i];
                  auto& dst_prop = dst_script.properties[i];
                  dst_prop.name = src_prop.name;
                  dst_prop.inheritance.present_on_base = true;
                  _copy_papyrus_property_value(src_prop.value, dst_prop.value);
               }
            }
         }
      }

      if (auto* this_papyrus = this->get_raw_papyrus_data()) {
         auto _get_or_insert = [&dst](const std::string& scriptname) -> auto& {
            for (auto& item : dst)
               if (item.name_matches(scriptname))
                  return item;
            auto& item = dst.emplace_back();
            item.scriptname = scriptname;
            return item;
         };

         for (auto& src_script : this_papyrus->scripts) {
            auto& dst_script = _get_or_insert(src_script.name);
            //
            if (src_script.status & components::papyrus::attached_script::status_flag::inherited) {
               dst_script.inheritance.present_on_target = true;
            }
            if (src_script.status & components::papyrus::attached_script::status_flag::removed) {
               dst_script.inheritance.removed_on_target = true;
               continue;
            }

            for (auto& src_prop : src_script.properties) {
               if (auto* dst_prop = dst_script.lookup_property(src_prop.name)) {
                  if (src_prop.status & components::papyrus::property::status_flag::inherited) {
                     dst_prop->inheritance.present_on_target = true;
                  }
                  if (src_prop.status & components::papyrus::property::status_flag::removed) {
                     dst_prop->inheritance.present_on_target = true; // "removed" without "inherited" is invalid
                     dst_prop->inheritance.removed_on_target = true;
                     break;
                  }
                  dst_prop->inheritance.present_on_target = true;
                  _copy_papyrus_property_value(src_prop.value, dst_prop->value);
                  continue;
               }
               auto& dst_prop = dst_script.properties.emplace_back();
               dst_prop.name = src_prop.name;
               dst_prop.inheritance.present_on_target = true;
               _copy_papyrus_property_value(src_prop.value, dst_prop.value);
            }
         }
      }

      return dst;
   }
   void Form::overwrite_papyrus_scripts(const std::vector<dovah::papyrus::scriptobject>& src) {
      auto* this_papyrus = this->get_raw_papyrus_data();
      assert(this_papyrus != nullptr);

      loaded_form_ptr<Form> base_form;
      const components::papyrus_attachment_data* base_papyrus = nullptr;
      //
      if (dovah::form_type_info::form_type_is_reference(this->stub.formType)) {
         auto* base_stub = form_stub_helpers::get_base_form(&this->stub);
         if (base_stub) {
            base_form = base_stub->load();
            if (base_form)
               base_papyrus = base_form->get_raw_papyrus_data();
         }
      }

      {  // Preserve fragment data; wipe everything else.
         auto* fragment_data = this_papyrus->fragment_data;
         this_papyrus->fragment_data = nullptr;
         this_papyrus->clear(*this);
         this_papyrus->fragment_data = fragment_data;
      }

      auto _copy_papyrus_property_value = [this](
         const dovah::papyrus::property_value& src,
         components::papyrus::property_value& dst
      ) {
         using backend_object_type  = components::papyrus::property_object_value;
         using frontend_object_type = dovah::papyrus::property_object_value;

         std::visit(
            [this, &dst](const auto& casted) {
               using value_type = std::decay_t<decltype(casted)>;
               if constexpr (std::is_same_v<value_type, frontend_object_type>) {
                  dst = backend_object_type{};
                  auto& dst_casted = std::get<backend_object_type>(dst);
                  
                  dst_casted.alias_id = casted.alias_id;
                  dst_casted.form.set(*this, casted.form);
               } else if constexpr (std::is_same_v<value_type, std::vector<frontend_object_type>>) {
                  dst = std::vector<backend_object_type>{};
                  auto& dst_casted = std::get<std::vector<backend_object_type>>(dst);

                  size_t size = casted.size();
                  dst_casted.resize(size);
                  for (size_t i = 0; i < size; ++i) {
                     dst_casted[i].alias_id = casted[i].alias_id;
                     dst_casted[i].form.set(*this, casted[i].form);
                  }
               } else {
                  dst = casted;
               }
            },
            src
         );
      };

      if (!base_papyrus) {
         // No base form. We can import the data more simply.
         for (auto& src_script : src) {
            auto& dst_script = this_papyrus->scripts.emplace_back();
            dst_script.name = src_script.scriptname;

            for (auto& src_prop : src_script.properties) {
               auto& dst_prop = dst_script.properties.emplace_back();
               dst_prop.name = src_prop.name;
               _copy_papyrus_property_value(src_prop.value, dst_prop.value);
            }
         }
      } else {
         for (auto& src_script : src) {
            const auto* inherited_script = base_papyrus->lookup_script(src_script.scriptname);

            if (src_script.inheritance.removed_on_target && !inherited_script)
               continue;

            auto& dst_script = this_papyrus->scripts.emplace_back();
            dst_script.name = src_script.scriptname;
            if (inherited_script) {
               dst_script.status |= components::papyrus::attached_script::status_flag::inherited;
               if (src_script.inheritance.removed_on_target) {
                  dst_script.status |= components::papyrus::attached_script::status_flag::removed;
                  continue;
               }
            }

            for (auto& src_prop : src_script.properties) {
               const components::papyrus::property* inherited_property = nullptr;
               if (inherited_script)
                  inherited_property = inherited_script->lookup_property(src_prop.name);

               if (src_prop.inheritance.removed_on_target && !inherited_property)
                  continue;

               auto& dst_prop = dst_script.properties.emplace_back();
               dst_prop.name = src_prop.name;
               dst_prop.status = components::papyrus::property::status_flag::altered;
               if (inherited_property) {
                  dst_prop.status = components::papyrus::property::status_flag::inherited;
                  if (src_prop.inheritance.removed_on_target) {
                     dst_prop.status = components::papyrus::property::status_flag::removed;
                     continue;
                  }
               }
               _copy_papyrus_property_value(src_prop.value, dst_prop.value);
            }

         }
      }
   }

   components::model* Form::get_model() noexcept {
      return component_access::get_model(this);
   }
   components::object_bounds* Form::get_object_bounds() noexcept {
      return component_access::get_object_bounds(this);
   }
   const components::papyrus_attachment_data* Form::get_raw_papyrus_data() const noexcept {
      return component_access::get_papyrus_data(const_cast<Form*>(this));
   }
   components::papyrus_attachment_data* Form::get_raw_papyrus_data() noexcept {
      return component_access::get_papyrus_data(this);
   }

   const char* Form::get_editor_id() const noexcept {
      return this->stub.get_editor_id();
   }
   void Form::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
   }
   Form* Form::clone(form_stub& receiving_stub, bool* out_complete) const noexcept {
      assert(receiving_stub.form == nullptr && "Cannot clone a loaded form into a stub that already has a loaded form.");
      //
      constructor_params fcp;
      fcp.stub = &receiving_stub;
      //
      auto instance = create_blank_loaded_form_by_type(this->formType, fcp);
      if (instance) {
         receiving_stub.form = instance;
         receiving_stub.set_edited(true);
         if (this->stub.addenda)
            receiving_stub.get_or_create_addenda().clone_from(*this->stub.addenda);
         bool result = this->_clone_impl(instance);
         if (out_complete)
            *out_complete = result;
      }
      return instance;
   }
   void Form::clear() {
      this->_clear_impl();
   }
   bool Form::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      assert(!this->is_working_copy && "Do not call Form::save on a working copy of a loaded form!");
      if (this->stub.is_deleted()) {
         //
         // Specific form types don't appear to save ANY data -- not even editor IDs -- if they 
         // are flagged as deleted.
         //
         auto& info = form_type_info::lookup(this->formType);
         if (info.flags & form_type_info::flag::empty_if_deleted)
            return true;
      }
      //
      auto editor_id = this->get_editor_id();
      if (editor_id && editor_id[0])
         record.write_string_subrecord('EDID', editor_id);
      //
      return this->_save_impl(record, intfc);
   }
   void Form::friendly_delete_override(const file_load_order& load_order) noexcept {
      bool flag = !this->_friendly_delete_impl(load_order);
      this->stub.edit_record_flags(form_flag::deleted, flag);
   }
   void Form::flag_as_deleted() noexcept {
      this->stub.edit_record_flags(form_flag::deleted, true);
   }
   void Form::sever_outbound_references_to(form_stub& other) noexcept {
      if (!this->is_working_copy) {
         if (auto* working = this->stub.working_copy)
            working->sever_outbound_references_to(other);
      }
      this->_sever_outbound_references_impl(other);
   }

   /*static*/ bool Form::subrecord_is_handled_elsewhere(uint32_t signature) {
      switch (signature) {
         case 'EDID':
            return true;
      }
      return false;
   }
}