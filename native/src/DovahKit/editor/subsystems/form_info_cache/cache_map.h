#pragma once
#include <mutex>
#include <QHash>
#include "dovah/core.h" // bare_form_id_t
#include "dovah/form_stub.h"

namespace dovahkit::subsystems::form_info_cache {
   //
   // A subclass of QHash that offers two things:
   // 
   //  - A thread-safe "threaded insert" function, for insertions during 
   //    initial load.
   // 
   //  - Convenience functions for insertions and removals post-load.
   //
   template<typename ValueType>
   class cache_map : public QHash<const dovah::form_stub*, ValueType> {
      private:
         std::mutex lock;

      public:
         void threadedInsert(const dovah::form_stub& stub, const ValueType& value) {
            auto guard = std::unique_lock(this->lock);
            this->insert(&stub, value);
         }
         void threadedInsert(const dovah::form_stub& stub, ValueType&& value) {
            auto guard = std::unique_lock(this->lock);
            this->insert(&stub, std::move(value));
         }

         // Returns `true` if there was an element to erase.
         bool eraseAndReport(const dovah::form_stub& stub) {
            auto it = this->find(&stub);
            if (it != this->end()) {
               this->erase(it);
               return true;
            }
            return false;
         }

         // Returns the removed element, if one was present.
         std::optional<ValueType> takeAndReport(const dovah::form_stub& stub) {
            auto it = this->find(&stub);
            if (it != this->end()) {
               auto result = *it;
               this->erase(it);
               return result;
            }
            return {};
         }

         bool replaceAndReport(const dovah::form_stub& stub, const ValueType& value) {
            auto it = this->find(&stub);
            if (it != this->end()) {
               if (*it == value) {
                  return false;
               }
               *it = value;
            } else {
               this->insert(&stub, value);
            }
            return true;
         }

         bool replaceTakeAndReport(const dovah::form_stub& stub, const ValueType& value, ValueType& out_prior_if_replaced) {
            auto it = this->find(&stub);
            if (it != this->end()) {
               if (*it == value) {
                  return false;
               }
               out_prior_if_replaced = *it;
               *it = value;
            } else {
               out_prior_if_replaced = ValueType{};
               this->insert(&stub, value);
            }
            return true;
         }

         const ValueType* valuePointer(const dovah::form_stub& stub) const {
            auto it = this->find(&stub);
            if (it != this->end())
               return &it.value();
            return nullptr;
         }
         ValueType* valuePointer(const dovah::form_stub& stub) {
            return const_cast<ValueType*>(std::as_const(*this).valuePointer(stub));
         }
   };
}