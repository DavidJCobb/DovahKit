#include "form_data_cache.h"
#include "core.h"
#include "../dovah/files/tes_file_reading/elements.h"
#include "../dovah/forms/Quest.h"
#include "../dovah/form_stub.h"
#include "../dovah/form_stub_addenda.h"
#include "form_data_cache_internals/threaded_builder.h"
#include <QVariant>

DovahKitFormDataCache::DovahKitFormDataCache() {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &DovahKitFormDataCache::clear);
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &DovahKitFormDataCache::buildAllData);
   QObject::connect(&editor, &DovahKitCore::formModified,         this, &DovahKitFormDataCache::handleFormChange);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DovahKitFormDataCache::handleFormDelete);
}

void DovahKitFormDataCache::handleFormChange(dovah::form_stub* stub) {
   dovah::bare_form_id_t form_id = stub->formID;
   switch (stub->formType) {
      case dovah::form_type::quest:
         {
            QString working;
            {
               auto form = stub->load().ptr_cast<dovah::loaded_forms::Quest>();
               assert(form);
               working = QString::fromStdString(form->filter);
            }
            //
            auto& set = this->_data.quest_filters.data;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               if (*it != working) {
                  QVariant prior = *it;
                  if (working.isEmpty()) {
                     set.erase(it);
                     emit this->cachedDataRemoved(stub, 'FLTR', prior);
                  } else {
                     QVariant after = working;
                     *it = working;
                     emit this->cachedDataChanged(stub, 'FLTR', prior, after);
                  }
               }
            } else if (!working.isEmpty()) {
               QVariant prior;
               QVariant after = working;
               set[form_id] = working;
               emit this->cachedDataChanged(stub, 'FLTR', prior, after);
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
void DovahKitFormDataCache::handleFormDelete(dovah::form_stub* stub, bool will_be_flagged) {
   dovah::bare_form_id_t form_id = stub->formID;
   switch (stub->formType) {
      case dovah::form_type::quest:
         {
            auto& set = this->_data.quest_filters.data;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               QVariant prior = *it;
               set.erase(it);
               emit this->cachedDataRemoved(stub, 'FLTR', prior);
            }
         }
         break;
      case dovah::form_type::statik:
         {
            auto& set = this->_data.static_models.data;
            auto  it  = set.find(form_id);
            if (it != set.end()) {
               QVariant prior = *it;
               set.erase(it);
               emit this->cachedDataRemoved(stub, 'MODL', prior);
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
      subrecord.read(raw);
      //
      auto& set   = store._data.quest_filters;
      auto  guard = std::lock_guard(set.lock);
      set.data[stub.formID] = QString::fromStdString(raw);
      break;
   }
}
/*static*/ void DovahKitFormDataCache::_parseStatic(dovah::form_stub& stub, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc) {
   if (!intfc.is_winning_record)
      return;
   while (auto& subrecord = record.next_subrecord()) {
      if (subrecord.signature() != 'MODL')
         continue;
      auto& store = DovahKitFormDataCache::get();
      std::string raw;
      subrecord.read(raw);
      //
      auto& set   = store._data.static_models;
      auto  guard = std::lock_guard(set.lock);
      set.data[stub.formID] = QString::fromStdString(raw);
      break;
   }
}

void DovahKitFormDataCache::buildAllData() {
   std::array<DovahKitEditorInternals::form_data_cache_builder, 8> builders;
   uint32_t count = 0;
   //
   auto& editor = DovahKitCore::get();
   editor.for_each_form_of_type(dovah::form_type::quest, [&builders, &count](dovah::form_stub* stub) {
      builders[count % builders.size()].add_to_queue(&DovahKitFormDataCache::_parseQuest, stub);
      ++count;
      return false;
   });
   this->_data.quest_filters.data.reserve(count / 1.5);
   //
   editor.for_each_form_of_type(dovah::form_type::statik, [&builders, &count](dovah::form_stub* stub) {
      builders[count % builders.size()].add_to_queue(&DovahKitFormDataCache::_parseStatic, stub);
      ++count;
      return false;
   });
   this->_data.static_models.data.reserve(count / 1.5);
   //
   for (auto& b : builders)
      b.start();
   for (auto& b : builders)
      b.wait_for();
   //
   emit this->cachedDataBuilt();
}

void DovahKitFormDataCache::clear() {
   this->_data.quest_filters.data.clear();
   this->_data.static_models.data.clear();
   //
   emit this->cachedDataCleared();
}

void DovahKitFormDataCache::forAllDataOfType(dovah::form_type_t ft, uint32_t code, std::function<bool(const QVariant&)> functor) const {
   if (ft == dovah::form_type::quest && code == 'FLTR') {
      for (auto& data : this->_data.quest_filters.data)
         if ((functor)(data))
            break;
      return;
   }
   if (ft == dovah::form_type::statik && code == 'MODL') {
      for (auto& data : this->_data.static_models.data)
         if ((functor)(data))
            break;
      return;
   }
}
QVariant DovahKitFormDataCache::dataFor(const dovah::form_stub* stub, uint32_t code) const {
   if (!stub || !code)
      return QVariant();
   auto ft = stub->formType;
   auto id = stub->formID;
   if (ft == dovah::form_type::quest && code == 'FLTR') {
      auto& set = this->_data.quest_filters.data;
      auto  it  = set.find(id);
      if (it != set.end())
         return *it;
      return QVariant();
   }
   if (code == 'MODL') {
      if (ft == dovah::form_type::statik) {
         auto& set = this->_data.static_models.data;
         auto  it  = set.find(id);
         if (it != set.end())
            return *it;
      }
      return QVariant();
   }
   return QVariant();
}