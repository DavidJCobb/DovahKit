#include "papyrus.h"
#include "../_common_cpp.h"
#include "../../logging.h"
#include <cassert>

namespace dovah::loaded_forms::components::papyrus {
   void script_data::for_each_script(std::function<bool(script_data::script*)> functor) {
      auto& list = this->scripts;
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (functor(&*it))
            break;
      }
   }
   script_data::property::~property() {
      if (this->value) {
         delete this->value;
         this->value = nullptr;
      }
   }
   bool script_data::load(tes_subrecord_reader& subrecord) {
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
         case 'QUST':
            this->fragment_data = (basic_fragment_data*) new quest_fragment_data;
            break;
         case 'SCEN':
            this->fragment_data = (basic_fragment_data*) new scene_fragment_data;
            break;
      }
      if (this->fragment_data)
         this->fragment_data->load(*this, subrecord);
      return subrecord.is_in_bounds();
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
   bool script_data::property_object_value::load(script_data& owner, tes_subrecord_reader& subrecord) {
      if (!subrecord.is_in_bounds(sizeof(this->always_zero) + sizeof(this->aliasID) + sizeof(this->formID)))
         return false;
      if (owner.object_format == 2) {
         subrecord.unchecked_read(this->always_zero);
         subrecord.unchecked_read(this->aliasID);
         subrecord.unchecked_read(this->formID);
      } else {
         subrecord.unchecked_read(this->formID);
         subrecord.unchecked_read(this->aliasID);
         subrecord.unchecked_read(this->always_zero);
      }
      return true;
   }
   bool script_data::property::load(script_data& owner, tes_subrecord_reader& subrecord) {
      //
      // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
      //
      subrecord.read_length_prefixed_string<2>(this->name);
      if (!subrecord.is_in_bounds(sizeof(this->type) + sizeof(this->status)))
         return false;
      subrecord.unchecked_read(this->type);
      subrecord.unchecked_read(this->status);
      if (this->value) {
         delete this->value;
         this->value = nullptr;
      }
      if (!property_type_is_array(this->type)) {
         switch (this->type) {
            case property_type::object:
               {
                  auto v = new property_object_value;
                  this->value = v;
                  v->load(owner, subrecord);
               }
               break;
            case property_type::string:
               {
                  auto v = new std::string;
                  this->value = v;
                  subrecord.read_length_prefixed_string<2>(*v);
               }
               break;
            case property_type::integer:
               {
                  auto v = new int32_t;
                  this->value = v;
                  subrecord.read(*v);
               }
               break;
            case property_type::float32:
               {
                  auto v = new float;
                  this->value = v;
                  subrecord.read(*v);
               }
               break;
            case property_type::boolean:
               {
                  static_assert(sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
                  auto v = new bool;
                  this->value = v;
                  subrecord.read(*v);
               }
               break;
            default:
               dovah::logging::print_line("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
               assert(false && "bad property type");
               return false;
         }
      } else {
         uint32_t count;
         if (!subrecord.read(count))
            return false;
         switch (this->type) {
            case property_type::array_of_object:
               {
                  auto v = new std::vector<property_object_value>;
                  this->value = v;
                  //
                  auto& values = *v;
                  values.resize(count);
                  for (uint32_t i = 0; i < count; i++) {
                     auto& elem = values[i];
                     elem.load(owner, subrecord);
                  }
               }
               break;
            case property_type::array_of_string:
               {
                  auto v = new std::vector<std::string>;
                  this->value = v;
                  //
                  auto& values = *v;
                  values.resize(count);
                  for (uint32_t i = 0; i < count; i++) {
                     auto& elem = values[i];
                     subrecord.read_length_prefixed_string<2>(elem);
                  }
               }
               break;
            case property_type::array_of_integer:
               {
                  if (!subrecord.is_in_bounds(sizeof(int32_t) * count))
                     return false;
                  auto v = new std::vector<int32_t>;
                  this->value = v;
                  //
                  auto& values = *v;
                  values.resize(count);
                  for (uint32_t i = 0; i < count; i++) {
                     auto& elem = values[i];
                     subrecord.unchecked_read(elem);
                  }
               }
               break;
            case property_type::array_of_float32:
               {
                  if (!subrecord.is_in_bounds(sizeof(float) * count))
                     return false;
                  auto v = new std::vector<float>;
                  this->value = v;
                  //
                  auto& values = *v;
                  values.resize(count);
                  for (uint32_t i = 0; i < count; i++) {
                     auto& elem = values[i];
                     subrecord.unchecked_read(elem);
                  }
               }
               break;
            case property_type::array_of_boolean:
               {
                  if (!subrecord.is_in_bounds(sizeof(uint8_t) * count))
                     return false;
                  auto v = new std::vector<bool>;
                  this->value = v;
                  //
                  auto& values = *v;
                  values.resize(count);
                  for (uint32_t i = 0; i < count; i++) {
                     uint8_t b;
                     subrecord.unchecked_read(b);
                     values[i] = (bool)b;
                  }
               }
               break;
            default:
               dovah::logging::print_line("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
               return false;
         }
      }
      return true;
   }
   #pragma endregion

   #pragma region Fragment data loading
   void topic_info_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) { // virtual
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
   void package_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) { // virtual
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
   void perk_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) { // virtual
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
   void quest_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) { // virtual
      subrecord.unchecked_read(this->unknown);
      uint16_t fragCount;
      subrecord.unchecked_read(fragCount);
      subrecord.read_length_prefixed_string<2>(this->filename);
      for (uint16_t i = 0; i < fragCount; i++) {
         this->fragments.emplace_back();
         auto& frag = *this->fragments.rbegin();
         if (!subrecord.is_in_bounds(5))
            break;
         subrecord.unchecked_read(frag.index);
         subrecord.unchecked_read(frag.unknown02);
         subrecord.unchecked_read(frag.logEntry);
         subrecord.unchecked_read(frag.unknown08);
         if (!subrecord.read_length_prefixed_string<2>(frag.filename))
            break;
         if (!subrecord.read_length_prefixed_string<2>(frag.function))
            break;
      }
      uint16_t aliasCount;
      subrecord.unchecked_read(aliasCount);
      for (uint16_t i = 0; i < aliasCount; i++) {
         this->aliasScriptData.emplace_back();
         auto& alias = *this->aliasScriptData.rbegin();
         alias.alias.load(owner, subrecord);
         subrecord.unchecked_read(alias.version);
         subrecord.unchecked_read(alias.objFormat);
         uint16_t scriptCount;
         subrecord.unchecked_read(scriptCount);
         for (uint16_t j = 0; j < scriptCount; j++) {
            alias.scripts.emplace_back();
            auto& script = *alias.scripts.rbegin();
            script.load(owner, subrecord);
         }
      }
   }
   void scene_fragment_data::load(script_data& owner, tes_subrecord_reader& subrecord) { // virtual
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
   #pragma endregion

   #pragma region Use info
   namespace {
      void _generateUseInfoForScript(int16_t objFormat, tes_subrecord_reader& subrecord, form_stub* stub) {
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
                     stub->add_outbound_reference(formID);
                  } else {
                     subrecord.read(formID);
                     stub->add_outbound_reference(formID);
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
                        stub->add_outbound_reference(formID);
                     } else {
                        subrecord.read(formID);
                        stub->add_outbound_reference(formID);
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
   /*static*/ void script_data::generateUseInfo(tes_subrecord_reader& subrecord, form_stub* stub) {
      form_id_t formID;
      //
      int16_t  objFormat;
      uint16_t count;
      subrecord.skip_bytes(2); // script version
      if (!subrecord.read(objFormat) || !subrecord.read(count))
         return;
      for (uint16_t i = 0; i < count; i++) // scripts
         _generateUseInfoForScript(objFormat, subrecord, stub);
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
                     stub->add_outbound_reference(formID);
                  } else {
                     subrecord.read(formID);
                     stub->add_outbound_reference(formID);
                     subrecord.skip_bytes(4);
                  }
                  subrecord.skip_bytes(2); // alias script version
                  uint16_t aliasObjFormat;
                  uint16_t aliasScriptCount;
                  if (!subrecord.read(aliasObjFormat) || !subrecord.read(aliasScriptCount))
                     return;
                  for(uint16_t j = 0; j < aliasScriptCount; j++)
                     _generateUseInfoForScript(aliasObjFormat, subrecord, stub);
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

