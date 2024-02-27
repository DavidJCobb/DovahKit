#pragma once
#include <mutex>
#include <QHash>
#include "dovah/core.h" // bare_form_id_t
#include "dovah/form_stub.h"

namespace dovahkit::subsystems::form_info_cache {
   template<typename ValueType>
   class cache_map : public QHash<dovah::bare_form_id_t, ValueType> {
      private:
         std::mutex lock;

      public:
         void threadedInsert(const dovah::form_stub& stub, const ValueType& value) {
            auto guard = std::unique_lock(this->lock);
            this->insert(stub.formID, value);
         }
         void threadedInsert(const dovah::form_stub& stub, ValueType&& value) {
            auto guard = std::unique_lock(this->lock);
            this->insert(stub.formID, std::move(value));
         }

         bool eraseAndReport(const dovah::form_stub& stub) {
            auto it = this->find(stub.formID);
            if (it != this->end()) {
               this->erase(it);
               return true;
            }
            return false;
         }
         bool replaceAndReport(const dovah::form_stub& stub, const ValueType& value) {
            auto it = this->find(stub.formID);
            if (it != this->end()) {
               if (*it == value) {
                  return false;
               }
               *it = value;
            } else {
               this->insert(stub.formID, value);
            }
            return true;
         }
         bool replaceAndReport(const dovah::form_stub& stub, ValueType&& value) {
            auto it = this->find(stub.formID);
            if (it != this->end()) {
               if (*it == value) {
                  return false;
               }
               *it = std::move(value);
            } else {
               this->insert(stub.formID, std::move(value));
            }
            return true;
         }
   };
}