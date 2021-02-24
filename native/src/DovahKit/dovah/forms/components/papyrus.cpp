#include "papyrus.h"
#include "../_common_cpp.h"
#include "../../logging.h"
#include <cassert>

namespace dovah::loaded_forms::components::papyrus {
   const script_data_save_parameters script_data_save_parameters::default = script_data_save_parameters();

   void script_data::for_each_script(std::function<bool(script_data::script*)> functor) {
      auto& list = this->scripts;
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (functor(&*it))
            break;
      }
   }
   //
   bool script_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (!subrecord.read(this->version) || !subrecord.read(this->object_format))
         return false;
      {
         uint16_t count;
         if (!subrecord.read(count))
            return false;
         this->scripts.resize(count);
         for (uint16_t i = 0; i < count; i++) {
            auto& script = this->scripts[i];
            if (!script.load(*this, subrecord))
               return false;
         }
      }
      if (subrecord.is_at_end()) // fragment data is optional
         return true;
      if (!subrecord.is_in_bounds())
         return false;
      switch (subrecord.containing_record_signature()) {
         case 'INFO':
            this->fragment_data = (basic_fragment_data*) new topic_info_fragment_data;
            break;
         case 'PACK':
            this->fragment_data = (basic_fragment_data*) new package_fragment_data;
            break;
         case 'PERK':
            this->fragment_data = (basic_fragment_data*) new perk_fragment_data;
            break;
         case 'SCEN':
            this->fragment_data = (basic_fragment_data*) new scene_fragment_data;
            break;
      }
      if (this->fragment_data)
         this->fragment_data->load(*this, subrecord);
      return subrecord.is_in_bounds();
   }
   bool script_data::save(tes_subrecord_writer& subrecord, save_interface_t& intfc, const script_data_save_parameters& params) {
      subrecord.write(this->version);
      subrecord.write(this->object_format);
      if (this->scripts.size() > std::numeric_limits<uint16_t>::max())
         return false;
      subrecord.write(uint16_t(this->scripts.size()));
      for (auto& script : this->scripts) {
         if (!script.save(*this, subrecord))
            return false;
      }
      if (this->fragment_data)
         this->fragment_data->save(*this, subrecord, params);
      return true;
   }
   bool script_data::save(tes_record_writer& record, save_interface_t& intfc, const script_data_save_parameters& params) {
      if (this->scripts.empty() && !this->fragment_data)
         return true;
      auto& VMAD = record.open_next_subrecord('VMAD');
      auto result = this->save(VMAD, intfc, params);
      VMAD.close();
      return result;
   }
   void script_data::clone_from(const script_data& other, loaded_forms::Form& owner_of_clone) noexcept {
      this->version       = other.version;
      this->object_format = other.object_format;
      //
      for (auto& script : this->scripts) { // clear any outbound use info we may have in here
         script.clear_properties(owner_of_clone);
      }
      this->scripts.clear();
      //
      size_t size = other.scripts.size();
      this->scripts.resize(size);
      for (size_t i = 0; i < size; ++i)
         this->scripts[i].clone_from(other.scripts[i], owner_of_clone);
      //
      if (this->fragment_data) {
         this->fragment_data->clear(owner_of_clone);
         delete this->fragment_data;
         this->fragment_data = nullptr;
      }
      if (other.fragment_data)
         this->fragment_data = other.fragment_data->clone(owner_of_clone);
   }
   void script_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& script : this->scripts)
         script.sever_outbound_references_to(target, my_owner);
      if (this->fragment_data)
         this->fragment_data->sever_outbound_references_to(target, my_owner);
   }
   void script_data::clear(loaded_forms::Form& my_owner) noexcept {
      for (auto& script : this->scripts)
         script.clear(my_owner);
      if (this->fragment_data)
         this->fragment_data->clear(my_owner);
   }

   #pragma region Script sub-objects loading
   bool script_data::script::load(script_data& owner, tes_subrecord_reader& subrecord) {
      subrecord.read_length_prefixed_string<2>(this->name);
      uint16_t count;
      if (!subrecord.is_in_bounds(sizeof(this->status) + sizeof(count)))
         return false;
      subrecord.unchecked_read(this->status);
      subrecord.unchecked_read(count);
      this->properties.resize(count);
      for (uint16_t i = 0; i < count; i++) {
         auto& prop = this->properties[i];
         if (!prop.load(owner, subrecord)) {
            dovah::logging::print_line("Problem encountered while loading property %d for script %s.", i, this->name.c_str());
            return false;
         }
      }
      return true;
   }
   bool script_data::script::save(script_data& owner, tes_subrecord_writer& subrecord) const noexcept {
      subrecord.write_length_prefixed_string<2>(this->name);
      uint16_t count = this->properties.size();
      if (this->properties.size() > std::numeric_limits<decltype(count)>::max()) {
         dovah::logging::print_line("Problem encountered while saving script %s: too many properties.", this->name.c_str());
         return false;
      }
      subrecord.write(this->status);
      subrecord.write(count);
      for (uint16_t i = 0; i < count; i++) {
         auto& prop = this->properties[i];
         if (!prop.save(owner, subrecord)) {
            dovah::logging::print_line("Problem encountered while saving property %d for script %s.", i, this->name.c_str());
            return false;
         }
      }
      return true;
   }
   void script_data::script::clear_properties(loaded_forms::Form& owner) {
      for (auto& prop : this->properties) {
         if (prop.type == property_type::object || prop.type == property_type::array_of_object) {
            for (auto& value : prop.values)
               value.object.clear(owner);
         }
      }
      this->properties.clear();
   }
   void script_data::script::clone_from(const script& other, loaded_forms::Form& owner_of_clone) noexcept {
      this->name   = other.name;
      this->status = other.status;
      //
      size_t size = other.properties.size();
      this->clear_properties(owner_of_clone);
      this->properties.resize(size);
      for (size_t i = 0; i < size; ++i) {
         this->properties[i].clone_from(other.properties[i], owner_of_clone);
      }
   }
   void script_data::script::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      for (auto& prop : this->properties)
         prop.sever_outbound_references_to(target, my_owner);
   }
   void script_data::script::clear(loaded_forms::Form& my_owner) noexcept {
      for (auto& prop : this->properties)
         prop.clear(my_owner);
   }

   bool script_data::property_object_value::load(script_data& owner, tes_subrecord_reader& subrecord) {
      if (!subrecord.is_in_bounds(sizeof(this->always_zero) + sizeof(this->aliasID) + sizeof(bare_form_id_t)))
         return false;
      if (owner.object_format == 2) {
         subrecord.unchecked_read(this->always_zero);
         subrecord.unchecked_read(this->aliasID);
         subrecord.unchecked_read(this->form);
      } else {
         subrecord.unchecked_read(this->form);
         subrecord.unchecked_read(this->aliasID);
         subrecord.unchecked_read(this->always_zero);
      }
      return true;
   }
   bool script_data::property_object_value::save(script_data& owner, tes_subrecord_writer& subrecord) const noexcept {
      if (owner.object_format == 2) {
         subrecord.write(this->always_zero);
         subrecord.write(this->aliasID);
         subrecord.write(this->form);
      } else {
         subrecord.write(this->form);
         subrecord.write(this->aliasID);
         subrecord.write(this->always_zero);
      }
      return true;
   }
   void script_data::property_object_value::clone_from(const property_object_value& other, loaded_forms::Form& owner) noexcept {
      this->form.set(owner, other.form);
      this->aliasID     = other.aliasID;
      this->always_zero = other.always_zero;
   }
   void script_data::property_object_value::clear(loaded_forms::Form& owner) {
      this->form.set(owner, nullptr);
   }
   void script_data::property_object_value::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->form.clear_if(my_owner, target);
   }

   bool script_data::property::value_t::load(property_type type, script_data& owner, tes_subrecord_reader& subrecord) {
      switch (type) {
            case property_type::object:
            case property_type::array_of_object:
               this->object.load(owner, subrecord);
               break;
            case property_type::string:
            case property_type::array_of_string:
               subrecord.read_length_prefixed_string<2>(this->string);
               break;
            case property_type::integer:
            case property_type::array_of_integer:
               subrecord.read(this->integer);
               break;
            case property_type::float32:
            case property_type::array_of_float32:
               subrecord.read(this->float32);
               break;
            case property_type::boolean:
            case property_type::array_of_boolean:
               static_assert(sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
               subrecord.read(this->boolean);
               break;
            default:
               return false;
      }
      return true;
   }
   bool script_data::property::value_t::save(property_type type, script_data& owner, tes_subrecord_writer& subrecord) const noexcept {
      switch (type) {
         case property_type::object:
         case property_type::array_of_object:
            this->object.save(owner, subrecord);
            break;
         case property_type::string:
         case property_type::array_of_string:
            subrecord.write_length_prefixed_string<2>(this->string);
            break;
         case property_type::integer:
         case property_type::array_of_integer:
            subrecord.write(this->integer);
            break;
         case property_type::float32:
         case property_type::array_of_float32:
            subrecord.write(this->float32);
            break;
         case property_type::boolean:
         case property_type::array_of_boolean:
            static_assert(sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
            subrecord.write(this->boolean);
            break;
         default:
            return false;
      }
      return true;
   }
   void script_data::property::value_t::clone_from(property_type type, const value_t& source, loaded_forms::Form& owner_of_clone) noexcept {
      switch (type) {
         case property_type::object:
         case property_type::array_of_object:
            this->object.clone_from(source.object, owner_of_clone);
            break;
         case property_type::string:
         case property_type::array_of_string:
            this->string = source.string;
            break;
         case property_type::integer:
         case property_type::array_of_integer:
            this->integer = source.integer;
            break;
         case property_type::float32:
         case property_type::array_of_float32:
            this->float32 = source.float32;
            break;
         case property_type::boolean:
         case property_type::array_of_boolean:
            this->boolean = source.boolean;
            break;
         default:
            assert(false && "Unable to clone Papyrus property value with an unrecognized type.");
      }
   }

   bool script_data::property::load(script_data& owner, tes_subrecord_reader& subrecord) {
      subrecord.read_length_prefixed_string<2>(this->name);
      if (!subrecord.is_in_bounds(sizeof(this->type) + sizeof(this->status)))
         return false;
      //
      assert(this->values.empty() && "We can't safely clear a Papyrus property's values list without access to the form stub, and we can't access the stub from here. Why is the list non-empty at the start of loading anyway?");
      //
      subrecord.unchecked_read(this->type);
      subrecord.unchecked_read(this->status);
      if (property_type_is_array(this->type)) {
         uint32_t count;
         if (!subrecord.read(count))
            return false;
         this->values.resize(count);
      } else {
         this->values.resize(1);
      }
      for (auto& value : this->values) {
         if (!value.load(this->type, owner, subrecord)) {
            dovah::logging::print_line("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
            assert(false && "bad property type");
            return false;
         }
      }
      //
      return true;
   }
   bool script_data::property::save(script_data& owner, tes_subrecord_writer& subrecord) const noexcept {
      subrecord.write_length_prefixed_string<2>(this->name);
      subrecord.write(this->type);
      subrecord.write(this->status);
      if (property_type_is_array(this->type)) {
         uint32_t count = this->values.size();
         subrecord.write(count);
      } else {
         assert(this->values.size() == 1 && "A non-array Papyrus property should not have more than one value.");
      }
      for (auto& value : this->values) {
         if (!value.save(this->type, owner, subrecord)) {
            dovah::logging::print_line("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
            assert(false && "bad property type");
            return false;
         }
      }
      return true;
   }
   void script_data::property::clone_from(const property& source, loaded_forms::Form& owner_of_clone) noexcept {
      if (this->type == property_type::object || this->type == property_type::array_of_object) {
         for (auto& value : this->values)
            value.object.clear(owner_of_clone);
      }
      this->values.clear();
      //
      this->name   = source.name;
      this->type   = source.type;
      this->status = source.status;
      //
      size_t size = source.values.size();
      this->values.resize(size);
      for (size_t i = 0; i < size; ++i) {
         this->values[i].clone_from(this->type, source.values[i], owner_of_clone);
      }
   }
   void script_data::property::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      switch (this->type) {
         case property_type::object:
         case property_type::array_of_object:
            break;
         default:
            return;
      }
      for (auto& value : this->values)
         value.object.sever_outbound_references_to(target, my_owner);
   }
   void script_data::property::clear(loaded_forms::Form& my_owner) noexcept {
      switch (this->type) {
         case property_type::object:
         case property_type::array_of_object:
            break;
         default:
            return;
      }
      for (auto& value : this->values)
         value.object.clear(my_owner);
   }
   #pragma endregion

   #pragma region Fragment data loading
   void topic_info_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) {
      if (subrecord.is_in_bounds(2)) {
         subrecord.unchecked_read(this->unknown);
         subrecord.unchecked_read(this->flags);
      }
      subrecord.read_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
   }
   void topic_info_fragment_data::save(script_data& owner, tes_subrecord_writer& subrecord, const script_data_save_parameters&) {
      subrecord.write(this->unknown);
      subrecord.write(this->flags);
      subrecord.write_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   basic_fragment_data* topic_info_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new topic_info_fragment_data;
      copy->unknown  = this->unknown;
      copy->flags    = this->flags;
      copy->filename = this->filename;
      copy->onBeginFragment = this->onBeginFragment;
      copy->onEndFragment   = this->onEndFragment;
      return copy;
   }

   void package_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) {
      if (subrecord.is_in_bounds(2)) {
         subrecord.unchecked_read(this->unknown);
         subrecord.unchecked_read(this->flags);
      }
      subrecord.read_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_change_fragment) {
         auto& frag = this->onChangeFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
   }
   void package_fragment_data::save(script_data& owner, tes_subrecord_writer& subrecord, const script_data_save_parameters&) {
      subrecord.write(this->unknown);
      subrecord.write(this->flags);
      subrecord.write_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_change_fragment) {
         auto& frag = this->onChangeFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   basic_fragment_data* package_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new package_fragment_data;
      copy->unknown  = this->unknown;
      copy->flags    = this->flags;
      copy->filename = this->filename;
      copy->onBeginFragment = this->onBeginFragment;
      copy->onEndFragment   = this->onEndFragment;
      copy->onChangeFragment = this->onChangeFragment;
      return copy;
   }

   void perk_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) {
      subrecord.read(this->unknown);
      subrecord.read_length_prefixed_string<2>(this->filename);
      uint16_t count = 0;
      if (subrecord.read(count)) {
         for (uint16_t i = 0; i < count; i++) {
            this->fragments.emplace_back();
            auto& frag = *this->fragments.rbegin();
            if (!subrecord.is_in_bounds(5))
               break;
            subrecord.unchecked_read(frag.index);
            subrecord.unchecked_read(frag.unknown02);
            subrecord.unchecked_read(frag.unknown04);
            if (!subrecord.read_length_prefixed_string<2>(frag.filename))
               break;
            if (!subrecord.read_length_prefixed_string<2>(frag.function))
               break;
         }
      }
   }
   void perk_fragment_data::save(script_data& owner, tes_subrecord_writer& subrecord, const script_data_save_parameters&) {
      subrecord.write(this->unknown);
      subrecord.write_length_prefixed_string<2>(this->filename);
      assert(this->fragments.size() <= std::numeric_limits<uint16_t>::max() && "Too many fragments in perk_fragment_data.");
      subrecord.write(uint16_t(this->fragments.size()));
      for (auto& frag : this->fragments) {
         subrecord.write(frag.index);
         subrecord.write(frag.unknown02);
         subrecord.write(frag.unknown04);
         subrecord.write_length_prefixed_string<2>(frag.filename);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   basic_fragment_data* perk_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new perk_fragment_data;
      copy->unknown  = this->unknown;
      copy->filename = this->filename;
      size_t size = this->fragments.size();
      copy->fragments.resize(size);
      for (size_t i = 0; i < size; ++i) {
         copy->fragments[i] = this->fragments[i];
      }
      return copy;
   }

   void scene_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) {
      if (subrecord.is_in_bounds(2)) {
         subrecord.unchecked_read(this->unknown);
         subrecord.unchecked_read(this->flags);
      }
      subrecord.read_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         if (subrecord.read(frag.unknown))
            if (subrecord.read_length_prefixed_string<2>(frag.script))
               subrecord.read_length_prefixed_string<2>(frag.function);
      }
      uint16_t count;
      if (subrecord.read(count)) {
         for (uint16_t i = 0; i < count; i++) {
            this->phaseFragments.emplace_back();
            auto& frag = *this->phaseFragments.rbegin();
            if (!subrecord.is_in_bounds(6))
               break;
            subrecord.unchecked_read(frag.unknown00);
            subrecord.unchecked_read(frag.phase);
            subrecord.unchecked_read(frag.unknown05);
            subrecord.read_length_prefixed_string<2>(frag.filename);
            subrecord.read_length_prefixed_string<2>(frag.function);
         }
      }
   }
   void scene_fragment_data::save(script_data& owner, tes_subrecord_writer& subrecord, const script_data_save_parameters&) {
      subrecord.write(this->unknown);
      subrecord.write(this->flags);
      subrecord.write_length_prefixed_string<2>(this->filename);
      if (this->flags & fragment_flag::has_begin_fragment) {
         auto& frag = this->onBeginFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      if (this->flags & fragment_flag::has_end_fragment) {
         auto& frag = this->onEndFragment;
         subrecord.write(frag.unknown);
         subrecord.write_length_prefixed_string<2>(frag.script);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
      assert(this->phaseFragments.size() <= std::numeric_limits<uint16_t>::max() && "Too many phase fragments in scene_fragment_data.");
      subrecord.write(uint16_t(this->phaseFragments.size()));
      for (auto& frag : this->phaseFragments) {
         subrecord.write(frag.unknown00);
         subrecord.write(frag.phase);
         subrecord.write(frag.unknown05);
         subrecord.write_length_prefixed_string<2>(frag.filename);
         subrecord.write_length_prefixed_string<2>(frag.function);
      }
   }
   basic_fragment_data* scene_fragment_data::clone(loaded_forms::Form& stub) const noexcept {
      auto* copy = new scene_fragment_data;
      copy->unknown  = this->unknown;
      copy->flags    = this->flags;
      copy->filename = this->filename;
      copy->onBeginFragment = this->onBeginFragment;
      copy->onEndFragment   = this->onEndFragment;
      //
      size_t size = this->phaseFragments.size();
      copy->phaseFragments.resize(size);
      for (size_t i = 0; i < size; ++i) {
         copy->phaseFragments[i] = this->phaseFragments[i];
      }
      return copy;
   }
   #pragma endregion

   #pragma region Use info
   namespace {
      void _generate_use_info_for_script(int16_t objFormat, tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
         form_id_t formID;
         //
         subrecord.skip_length_prefixed_string<2>();
         subrecord.skip_bytes(1); // script status
         uint16_t prop_count;
         if (!subrecord.read(prop_count))
            return;
         for (uint16_t j = 0; j < prop_count; j++) { // script properties
            subrecord.skip_length_prefixed_string<2>();
            //
            property_type type;
            subrecord.read(type);
            subrecord.skip_bytes(1); // property status
            uint32_t value_count = 1;
            if (property_type_is_array(type)) {
               if (!subrecord.read(value_count))
                  return;
            }
            switch (type) {
               case property_type::object:
                  if (objFormat == 2) {
                     subrecord.skip_bytes(4);
                     subrecord.read(formID);
                     uib.add_outbound_reference(formID);
                  } else {
                     subrecord.read(formID);
                     uib.add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                  }
                  break;
               case property_type::string:
                  subrecord.skip_length_prefixed_string<2>();
                  break;
               case property_type::integer:
               case property_type::float32:
                  subrecord.skip_bytes(4);
                  break;
               case property_type::boolean:
                  subrecord.skip_bytes(1);
                  break;
               case property_type::array_of_object:
                  for (uint32_t k = 0; k < value_count; k++) {
                     if (objFormat == 2) {
                        subrecord.skip_bytes(4);
                        subrecord.read(formID);
                        uib.add_outbound_reference(formID);
                     } else {
                        subrecord.read(formID);
                        uib.add_outbound_reference(formID);
                        subrecord.skip_bytes(4);
                     }
                  }
                  break;
               case property_type::array_of_string:
                  for (uint32_t k = 0; k < value_count; k++)
                     subrecord.skip_length_prefixed_string<2>();
                  break;
               case property_type::array_of_integer:
               case property_type::array_of_float32:
                  subrecord.skip_bytes(4 * value_count);
                  break;
               case property_type::array_of_boolean:
                  subrecord.skip_bytes(value_count);
                  break;
            }
         }
      }
   }
   /*static*/ void script_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      //
      int16_t  objFormat;
      uint16_t count;
      subrecord.skip_bytes(2); // script version
      if (!subrecord.read(objFormat) || !subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; i++) // scripts
         _generate_use_info_for_script(objFormat, subrecord, uib);
      if (subrecord.is_at_end() || !subrecord.is_in_bounds()) // fragment data is optional
         return;
      #if PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD != 1
         uint8_t flags;
      #endif
      switch (subrecord.containing_record_signature()) {
         case 'INFO': // should match topic_info_fragment_data::load
            #if PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD != 1
               {
                  using Flags = topic_info_fragment_data::fragment_flag;
                  //
                  subrecord.skip_bytes(1);
                  subrecord.unchecked_read(flags);
                  subrecord.skip_length_prefixed_string<2>();
                  if (flags & Flags::has_begin_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
                  if (flags & Flags::has_end_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
               }
            #endif
            break;
         case 'PACK': // should match package_fragment_data::load
            #if PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD != 1
               {
                  using Flags = package_fragment_data::fragment_flag;
                  //
                  subrecord.skip_bytes(1);
                  subrecord.unchecked_read(flags);
                  subrecord.skip_length_prefixed_string<2>();
                  if (flags & Flags::has_begin_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
                  if (flags & Flags::has_end_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
                  if (flags & Flags::has_change_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
               }
            #endif
            break;
         case 'PERK': // should match perk_fragment_data::load
            #if PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD != 1
               {
                  subrecord.skip_bytes(1);
                  subrecord.skip_length_prefixed_string<2>();
                  if (subrecord.read(count)) {
                     for (uint16_t i = 0; i < count; i++) {
                        subrecord.skip_bytes(5);
                        subrecord.skip_length_prefixed_string<2>();
                        subrecord.skip_length_prefixed_string<2>();
                     }
                  }
               }
            #endif
            break;
         case 'QUST': // should match quest_fragment_data::load
            {
               subrecord.skip_bytes(1);
               if (!subrecord.read(count)) // fragment count
                  return;
               subrecord.skip_length_prefixed_string<2>();
               for (uint16_t i = 0; i < count; i++) { // fragments
                  subrecord.skip_bytes(9);
                  subrecord.skip_length_prefixed_string<2>();
                  subrecord.skip_length_prefixed_string<2>();
               }
               if (!subrecord.read(count)) // alias script count
                  return;
               for (uint16_t i = 0; i < count; i++) { // alias scripts
                  if (objFormat == 2) {
                     subrecord.skip_bytes(4);
                     subrecord.read(formID);
                     uib.add_outbound_reference(formID);
                  } else {
                     subrecord.read(formID);
                     uib.add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                  }
                  subrecord.skip_bytes(2); // alias script version
                  uint16_t aliasObjFormat;
                  uint16_t aliasScriptCount;
                  if (!subrecord.read(aliasObjFormat) || !subrecord.read(aliasScriptCount))
                     return;
                  for(uint16_t j = 0; j < aliasScriptCount; j++)
                     _generate_use_info_for_script(aliasObjFormat, subrecord, uib);
               }
            }
            break;
         case 'SCEN': // should match scene_fragment_data::load
            #if PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD != 1
               {
                  using Flags = scene_fragment_data::fragment_flag;
                  //
                  subrecord.skip_bytes(1);
                  subrecord.unchecked_read(flags);
                  subrecord.skip_length_prefixed_string<2>();
                  if (flags & Flags::has_begin_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
                  if (flags & Flags::has_end_fragment) {
                     subrecord.skip_bytes(1);
                     subrecord.skip_length_prefixed_string<2>();
                     subrecord.skip_length_prefixed_string<2>();
                  }
                  if (subrecord.read(count)) {
                     for (uint16_t i = 0; i < count; i++) {
                        subrecord.skip_bytes(6);
                        subrecord.skip_length_prefixed_string<2>();
                        subrecord.skip_length_prefixed_string<2>();
                     }
                  }
               }
            #endif
            break;
      }
   }
   #pragma endregion
}

