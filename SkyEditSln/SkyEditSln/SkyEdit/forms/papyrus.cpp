#include "papyrus.h"
#include "../esp/TESPlugin.h"
#include "../output.h"

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

bool PapyrusScriptData::load(TESPluginFile* file) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   file->read(this->version);
   file->read(this->objectFormat);
   {
      uint16_t count;
      file->read(count);
      this->scripts.resize(count);
      for (uint16_t i = 0; i < count; i++) {
         auto& script = this->scripts[i];
         script.load(*this, file);
      }
   }
   switch (file->getRecordHeader().signature) {
      case 'INFO':
         //
         // TODO: fragment data for topic infos
         //
         break;
      case 'PACK':
         //
         // TODO: fragment data for packages
         //
         break;
      case 'PERK':
         //
         // TODO: fragment data for perks
         //
         break;
      case 'QUST':
         //
         // TODO: fragment data for quests
         //
         break;
      case 'SCEN':
         //
         // TODO: fragment data for scenes
         //
         break;
   }
   return true;
}
bool PapyrusScriptData::Script::load(PapyrusScriptData& owner, TESPluginFile* file) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   file->readWString(this->name);
   file->read(this->status);
   uint16_t count;
   file->read(count);
   this->properties.resize(count);
   for (uint16_t i = 0; i < count; i++) {
      auto& prop = this->properties[i];
      if (!prop.load(owner, file)) {
         _DEBUGMSG("Problem encountered while loading script %s.", this->name.c_str());
         return false;
      }
   }
   return true;
}
bool PapyrusScriptData::PropertyObjectValue::load(PapyrusScriptData& owner, TESPluginFile* file) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   if (owner.objectFormat == 2) {
      file->read(this->alwaysZero);
      file->read(this->aliasID);
      file->read(this->formID);
   } else {
      file->read(this->formID);
      file->read(this->aliasID);
      file->read(this->alwaysZero);
   }
   return true;
}
bool PapyrusScriptData::Property::load(PapyrusScriptData& owner, TESPluginFile* file) {
   //
   // TODO: Add some way to detect when we blow past the end of the VMAD subrecord and return false
   //
   file->readWString(this->name);
   file->read(this->type);
   file->read(this->status);
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
               v->load(owner, file);
            }
            break;
         case kPapyrusPropertyType_String:
            {
               auto v = new std::string;
               this->value = v;
               file->readWString(*v);
            }
            break;
         case kPapyrusPropertyType_Int:
            {
               auto v = new int32_t;
               this->value = v;
               file->read(*v);
            }
            break;
         case kPapyrusPropertyType_Float:
            {
               auto v = new float;
               this->value = v;
               file->read(*v);
            }
            break;
         case kPapyrusPropertyType_Bool:
            {
               static_assert(sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
               auto v = new bool;
               this->value = v;
               file->read(*v);
            }
            break;
         default:
            _DEBUGMSG("Property %s has unrecognized type %d.", this->name.c_str(), this->type);
            return false;
      }
   } else {
      uint32_t count;
      file->read(count);
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
                  elem.load(owner, file);
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
                  file->readWString(elem);
               }
            }
            break;
         case kPapyrusPropertyType_ArrayInt:
            {
               auto v = new std::vector<int32_t>;
               this->value = v;
               //
               auto& values = *v;
               values.resize(count);
               for (uint32_t i = 0; i < count; i++) {
                  auto& elem = values[i];
                  file->read(elem);
               }
            }
            break;
         case kPapyrusPropertyType_ArrayFloat:
            {
               auto v = new std::vector<float>;
               this->value = v;
               //
               auto& values = *v;
               values.resize(count);
               for (uint32_t i = 0; i < count; i++) {
                  auto& elem = values[i];
                  file->read(elem);
               }
            }
            break;
         case kPapyrusPropertyType_ArrayBool:
            {
               auto v = new std::vector<bool>;
               this->value = v;
               //
               auto& values = *v;
               values.resize(count);
               for (uint32_t i = 0; i < count; i++) {
                  uint8_t b;
                  file->read(b);
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