#pragma once
#include "./data_cache.h"
#if _DEBUG
   #include <cassert>
#endif

namespace dovahkit::subsystems::form_info_cache {
   template<typename ValueType>
   data_cache<ValueType>::data_cache() {
      #if _DEBUG
         this->_owning_thread = std::this_thread::get_id();
      #endif
   }

   template<typename ValueType>
   void data_cache<ValueType>::_verify_thread_affinity() const {
      #if _DEBUG
         assert(std::this_thread::get_id() == this->_owning_thread && "FIC data cache maps are only safe to access on their owning thread or for a *thread-safe* insertion specifically.");
      #endif
   }

   template<typename ValueType>
   void data_cache<ValueType>::threaded_insert(key_reference key, const value_type& value) {
      auto guard = std::unique_lock(this->_lock);
      this->_map.insert(&key, value);
   }

   template<typename ValueType>
   void data_cache<ValueType>::threaded_insert(key_reference key, value_type&& value) {
      auto guard = std::unique_lock(this->_lock);
      this->_map.insert(&key, std::move(value));
   }

   template<typename ValueType>
   void data_cache<ValueType>::insert(key_reference key, const value_type& value) {
      _verify_thread_affinity();
      this->_map.insert(&key, value);
   }

   template<typename ValueType>
   void data_cache<ValueType>::insert(key_reference key, value_type&& value) {
      _verify_thread_affinity();
      this->_map.insert(&key, value);
   }

   template<typename ValueType>
   bool data_cache<ValueType>::erase(key_reference key) {
      _verify_thread_affinity();
      auto it = this->_map.find(&key);
      if (it != this->_map.end()) {
         this->_map.erase(it);
         return true;
      }
      return false;
   }

   template<typename ValueType>
   const data_cache<ValueType>::value_type* data_cache<ValueType>::get(key_reference key) const {
      _verify_thread_affinity();
      auto it = this->_map.find(&key);
      if (it != this->_map.end())
         return &(*it);
      return nullptr;
   }

   template<typename ValueType>
   std::optional<typename data_cache<ValueType>::value_type> data_cache<ValueType>::take(key_reference key) {
      _verify_thread_affinity();
      auto it = this->_map.find(&key);
      if (it != this->_map.end()) {
         auto result = *it;
         this->_map.erase(it);
         return result;
      }
      return {};
   }

   template<typename ValueType>
   bool data_cache<ValueType>::replace(key_reference key, const value_type& value) {
      _verify_thread_affinity();
      auto it = this->_map.find(&key);
      if (it != this->_map.end()) {
         if (*it == value) {
            return false;
         }
         *it = value;
      } else {
         this->_map.insert(&key, value);
      }
      return true;
   }

   template<typename ValueType>
   std::optional<typename data_cache<ValueType>::value_type> data_cache<ValueType>::take_and_replace(key_reference key, const value_type& value) {
      _verify_thread_affinity();
      auto it = this->_map.find(&key);
      if (it == this->_map.end()) {
         this->_map.insert(&key, value);
         return value;
      }
      if (*it == value) {
         return {};
      }
      auto prior = *it;
      *it = value;
      return prior;
   }
}
