#include "papyrus.h"
#include "../esp/TESPlugin.h"
#include "../output.h"
#include <cassert>

void PapyrusScriptData::forEachScript(std::function<bool(PapyrusScriptData::Script*)> functor) {
   auto& list = this->scripts;
   for (auto it = list.begin(); it != list.end(); ++it) {
      if (functor(&*it))
         break;
   }
}

PapyrusScriptData::Property::~Property() {
   if (this->value) {
      delete this->value;
      this->value = nullptr;
   }
}

bool PapyrusScriptData::load(TESPluginSubrecord& subrecord) {
   if (!subrecord.read(this->version) || !subrecord.read(this->objectFormat))
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
         this->fragmentData = (PapyrusFragmentData*) new PapyrusTopicInfoFragmentData;
         break;
      case 'PACK':
         this->fragmentData = (PapyrusFragmentData*) new PapyrusPackageFragmentData;
         break;
      case 'PERK':
         this->fragmentData = (PapyrusFragmentData*) new PapyrusPerkFragmentData;
         break;
      case 'QUST':
         this->fragmentData = (PapyrusFragmentData*) new PapyrusQuestFragmentData;
         break;
      case 'SCEN':
         this->fragmentData = (PapyrusFragmentData*) new PapyrusSceneFragmentData;
         break;
   }
   if (this->fragmentData)
      this->fragmentData->load(*this, subrecord);
   return subrecord.is_in_bounds();
}
bool PapyrusScriptData::Script::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   subrecord.read_wstring(this->name);
   uint16_t count;
   if (!subrecord.is_in_bounds(sizeof(this->status) + sizeof(count)))
      return false;
   subrecord.unchecked_read(this->status);
   subrecord.unchecked_read(count);
   this->properties.resize(count);
   for (uint16_t i = 0; i < count; i++) {
      auto& prop = this->properties[i];
      if (!prop.load(owner, subrecord)) {
         _DEBUGMSG("Problem encountered while loading property %d for script %s.", i, this->name.c_str());
         return false;
      }
   }
   return true;
}
bool PapyrusScriptData::PropertyObjectValue::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) {
   if (!subrecord.is_in_bounds(sizeof(this->alwaysZero) + sizeof(this->aliasID) + sizeof(this->formID)))
      return false;
   if (owner.objectFormat == 2) {
      subrecord.unchecked_read(this->alwaysZero);
      subrecord.unchecked_read(this->aliasID);
      subrecord.unchecked_read(this->formID);
   } else {
      subrecord.unchecked_read(this->formID);
      subrecord.unchecked_read(this->aliasID);
      subrecord.unchecked_read(this->alwaysZero);
   }
   return true;
}
bool PapyrusScriptData::Property::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   subrecord.read_wstring(this->name);
   if (!subrecord.is_in_bounds(sizeof(this->type) + sizeof(this->status)))
      return false;
   subrecord.unchecked_read(this->type);
   subrecord.unchecked_read(this->status);
   if (this->value) {
      delete this->value;
      this->value = nullptr;
   }
   if (this->type < 11) {
      switch (this->type) {
         case kPapyrusPropertyType_Object:
            {
               auto v = new PropertyObjectValue;
               this->value = v;
               v->load(owner, subrecord);
            }
            break;
         case kPapyrusPropertyType_String:
            {
               auto v = new std::string;
               this->value = v;
               subrecord.read_wstring(*v);
            }
            break;
         case kPapyrusPropertyType_Int:
            {
               auto v = new int32_t;
               this->value = v;
               subrecord.read(*v);
            }
            break;
         case kPapyrusPropertyType_Float:
            {
               auto v = new float;
               this->value = v;
               subrecord.read(*v);
            }
            break;
         case kPapyrusPropertyType_Bool:
            {
               static_assert(sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
               auto v = new bool;
               this->value = v;
               subrecord.read(*v);
            }
            break;
         default:
            _DEBUGMSG("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
            assert(false && "bad property type");
            return false;
      }
   } else {
      uint32_t count;
      if (!subrecord.read(count))
         return false;
      switch (this->type) {
         case kPapyrusPropertyType_ArrayObject:
            {
               auto v = new std::vector<PropertyObjectValue>;
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
         case kPapyrusPropertyType_ArrayString:
            {
               auto v = new std::vector<std::string>;
               this->value = v;
               //
               auto& values = *v;
               values.resize(count);
               for (uint32_t i = 0; i < count; i++) {
                  auto& elem = values[i];
                  subrecord.read_wstring(elem);
               }
            }
            break;
         case kPapyrusPropertyType_ArrayInt:
            {
               if (!subrecord.is_in_bounds(sizeof(uint32_t) * count))
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
         case kPapyrusPropertyType_ArrayFloat:
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
         case kPapyrusPropertyType_ArrayBool:
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
            _DEBUGMSG("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
            return false;
      }
   }
   return true;
}

void PapyrusTopicInfoFragmentData::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) { // virtual
   if (subrecord.is_in_bounds(2)) {
      subrecord.unchecked_read(this->unknown);
      subrecord.unchecked_read(this->flags);
   }
   subrecord.read_length_prefixed_string<2>(this->filename);
   if (this->flags & Flags::has_begin_fragment) {
      auto& frag = this->onBeginFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
   if (this->flags & Flags::has_end_fragment) {
      auto& frag = this->onEndFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
}
void PapyrusPackageFragmentData::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) { // virtual
   if (subrecord.is_in_bounds(2)) {
      subrecord.unchecked_read(this->unknown);
      subrecord.unchecked_read(this->flags);
   }
   subrecord.read_length_prefixed_string<2>(this->filename);
   if (this->flags & Flags::has_begin_fragment) {
      auto& frag = this->onBeginFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
   if (this->flags & Flags::has_end_fragment) {
      auto& frag = this->onEndFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
   if (this->flags & Flags::has_change_fragment) {
      auto& frag = this->onChangeFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
}
void PapyrusPerkFragmentData::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) { // virtual
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
void PapyrusQuestFragmentData::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) { // virtual
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
void PapyrusSceneFragmentData::load(PapyrusScriptData& owner, TESPluginSubrecord& subrecord) { // virtual
   if (subrecord.is_in_bounds(2)) {
      subrecord.unchecked_read(this->unknown);
      subrecord.unchecked_read(this->flags);
   }
   subrecord.read_length_prefixed_string<2>(this->filename);
   if (this->flags & Flags::has_begin_fragment) {
      auto& frag = this->onBeginFragment;
      if (subrecord.read(frag.unknown))
         if (subrecord.read_length_prefixed_string<2>(frag.script))
            subrecord.read_length_prefixed_string<2>(frag.function);
   }
   if (this->flags & Flags::has_end_fragment) {
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