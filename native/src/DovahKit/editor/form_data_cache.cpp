#include "form_data_cache.h"
#include "core.h"
#include "../dovah/files/tes_file_reading/elements.h"
#include "../dovah/forms/Quest.h"
#include "../dovah/form_stub.h"
#include "../dovah/form_stub_addenda.h"
#include <QVariant>

constexpr bool quest_filters_are_in_addenda = true;

DovahKitFormDataCache::DovahKitFormDataCache() {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &DovahKitFormDataCache::clear);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &DovahKitFormDataCache::buildAllData);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &DovahKitFormDataCache::handleFormChange);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DovahKitFormDataCache::handleFormDelete);
}

void DovahKitFormDataCache::handleFormChange(dovah::form_stub& stub) {
   dovah::bare_form_id_t form_id = stub.formID;
   switch (stub.formType) {
      case dovah::form_type::quest:
         {
            QString working;
            if (quest_filters_are_in_addenda) {
               if (stub.addenda)
                  working = QString::fromStdString(stub.addenda->filter);
            } else {
               auto form = stub.load().ptr_cast<dovah::loaded_forms::Quest>();
               assert(form);
               //
               // ... TODO ...
               //
            }
            //
            auto& set = this->_data.quest_filters;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               if (*it != working) {
                  QVariant prior = *it;
                  if (working.isEmpty()) {
                     set.erase(it);
                     emit this->cachedDataRemoved(&stub, 'FLTR', prior);
                  } else {
                     QVariant after = working;
                     *it = working;
                     emit this->cachedDataChanged(&stub, 'FLTR', prior, after);
                  }
               }
            } else if (!working.isEmpty()) {
               QVariant prior;
               QVariant after = working;
               set[form_id] = working;
               emit this->cachedDataChanged(&stub, 'FLTR', prior, after);
            }
         }
         break;
      case dovah::form_type::statik:
         //
         // ... TODO ...
         //
         break;
   }
}
void DovahKitFormDataCache::handleFormDelete(dovah::form_stub& stub, bool will_be_flagged) {
   dovah::bare_form_id_t form_id = stub.formID;
   switch (stub.formType) {
      case dovah::form_type::quest:
         {
            auto& set = this->_data.quest_filters;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               QVariant prior = *it;
               set.erase(it);
               emit this->cachedDataRemoved(&stub, 'FLTR', prior);
            }
         }
         break;
      case dovah::form_type::statik:
         {
            auto& set = this->_data.static_models;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               QVariant prior = *it;
               set.erase(it);
               emit this->cachedDataRemoved(&stub, 'MODT', prior);
            }
         }
         break;
   }
}

/*static*/ void DovahKitFormDataCache::_parseQuest(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
   if (!intfc.is_winning_record)
      return;
   while (auto& subrecord = record.next_subrecord()) {
      if (subrecord.signature() != 'FLTR')
         continue;
      auto& store = DovahKitFormDataCache::get();
      std::string raw;
      subrecord.to_string(raw);
      store._data.quest_filters[stub.formID] = QString::fromStdString(raw);
      break;
   }
}
/*static*/ void DovahKitFormDataCache::_parseStatic(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
   if (!intfc.is_winning_record)
      return;
   while (auto& subrecord = record.next_subrecord()) {
      if (subrecord.signature() != 'MODT')
         continue;
      auto& store = DovahKitFormDataCache::get();
      std::string raw;
      subrecord.to_string(raw);
      store._data.static_models[stub.formID] = QString::fromStdString(raw);
      break;
   }
}

void DovahKitFormDataCache::buildAllData() {
   auto& editor = DovahKitCore::get();
   editor.for_each_form_of_type(dovah::form_type::quest, [this](dovah::form_stub* stub) {
      stub->do_custom_parse(&DovahKitFormDataCache::_parseQuest);
      return false;
   });
   editor.for_each_form_of_type(dovah::form_type::statik, [this](dovah::form_stub* stub) {
      stub->do_custom_parse(&DovahKitFormDataCache::_parseStatic);
      return false;
   });
   emit this->cachedDataBuilt();
}

void DovahKitFormDataCache::clear() {
   this->_data.quest_filters.clear();
   this->_data.static_models.clear();
   //
   emit this->cachedDataCleared();
}