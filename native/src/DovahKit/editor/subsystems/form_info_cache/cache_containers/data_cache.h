#pragma once
#include <mutex>
#include <optional>
#if _DEBUG
   #include <thread>
#endif
#include <QHash>

namespace dovah {
   class form_stub;
}

namespace dovahkit::subsystems::form_info_cache {
   //
   // A map of form stub pointers to an arbitrary value type. This map allows 
   // for thread-safe insertions. No other operations are thread-safe.
   //
   template<typename ValueType>
   class data_cache {
      public:
         using key_pointer   = const dovah::form_stub*;
         using key_reference = const dovah::form_stub&;
         using value_type    = ValueType;

      private:
         std::mutex _lock;
         QHash<key_pointer, value_type> _map;

         #if _DEBUG
            std::thread::id _owning_thread;
         #endif
         void _verify_thread_affinity() const;

      public:
         data_cache();
         //
         // std::mutex isn't copyable, so default construct/assign behavior e.g. (*this = {}) has 
         // to be defined manually:
         //
         data_cache(const data_cache& src) : data_cache() {
            *this = src;
         }
         data_cache(data_cache&& src) noexcept : data_cache() {
            *this = std::move(src);
         }
         data_cache& operator=(const data_cache& src) {
            this->_map = src._map;
            return *this;
         }
         data_cache& operator=(data_cache&& src) noexcept {
            this->_map = std::move(src._map);
            return *this;
         }

         inline void reserve(size_t s) {
            this->_map.reserve(s);
         }

         void threaded_insert(key_reference, const value_type&);
         void threaded_insert(key_reference, value_type&&);

         void insert(key_reference, const value_type&);
         void insert(key_reference, value_type&&);

         // Returns `true` if there was an element to erase.
         bool erase(key_reference);

         const value_type* get(key_reference key) const;
         value_type* get(key_reference key) {
            return const_cast<value_type*>(std::as_const(*this).get(key));
         }

         // Removes an element, if one is present, and returns it.
         std::optional<value_type> take(key_reference);

         // Replaces an element, returning true if any changes are made or false if 
         // the new value is identical to a preexisting value.
         bool replace(key_reference stub, const value_type& value);

         // Replaces an element. If there is a preexisting value and it differs from 
         // the passed-in value, then the preexisting value is returned.
         std::optional<value_type> take_and_replace(key_reference stub, const value_type& value);

         // Invokes a functor on each pair in the map, passing a form-stub reference 
         // and a value reference. Your functor can return void, or it can return bool; 
         // in the latter case, we stop iteration early when you return true.
         template<typename Functor> requires std::is_invocable_v<Functor, key_reference, const value_type&>
         void for_each(Functor&& functor) const {
            constexpr bool returns_bool = std::is_same_v<bool, std::invoke_result_t<Functor, key_reference, const value_type&>>;

            for (auto it = this->_map.keyValueBegin(); it != this->_map.keyValueEnd(); ++it) {
               if constexpr (returns_bool) {
                  bool finished_early = functor(*it->first, it->second);
                  if (finished_early)
                     break;
               } else {
                  functor(*it->first, it->second);
               }
            }
         }
         
         template<typename Functor> requires std::is_invocable_v<Functor, key_reference, value_type&>
         void for_each(Functor&& functor) {
            constexpr bool returns_bool = std::is_same_v<bool, std::invoke_result_t<Functor, key_reference, value_type&>>;

            for (auto it = this->_map.keyValueBegin(); it != this->_map.keyValueEnd(); ++it) {
               if constexpr (returns_bool) {
                  bool finished_early = functor(*it->first, it->second);
                  if (finished_early)
                     break;
               } else {
                  functor(*it->first, it->second);
               }
            }
         }
   };
}

#include "./data_cache.inl"