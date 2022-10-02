#include "_factory.h"
#include <typeinfo>
//
#include "_all.h"

namespace nifDK {
   extern bool block_is_of_type(block* b, const std::string& type_name) {
      if (!b)
         return false;
      bool matches = false;
      all_block_types::for_each_until_true(
         [&type_name, &matches]<typename T>(const block* b) {
            if constexpr (block_type_has_name<T>) {
               if (type_name == T::type_name) {
                  matches = (dynamic_cast<const T*>(b) != nullptr);
                  return true;
               }
            }
            return false;
         },
         b
      );
      return matches;
   }
   extern block* create_block_of_type(const std::string& type_name) {
      block* out     = nullptr;
      bool   matched = all_block_types::for_each_until_true(
         [&type_name, &out]<typename T>() {
            if constexpr (block_type_has_name<T>) {
               if (type_name == T::type_name) {
                  if constexpr (std::is_abstract_v<T>) {
                     out = nullptr;
                  } else {
                     out = new T();
                  }
                  return true;
               }
            }
            return false;
         }
      );
      if (out)
         return out;
      if (matched) {
         //
         // We *found* a matching class, but could not construct it; it's probably abstract.
         //
         return nullptr;
      }
      return new block_types::unknown_block(type_name);
   }

   extern const char* get_block_typename(const block& b) {
      const char* result = nullptr;
      //
      auto& type = typeid(b);
      all_block_types::for_each_until_true(
         [&type, &result]<typename T>() {
            if constexpr (block_type_has_name<T>) {
               if (typeid(T) == type) {
                  result = T::type_name;
                  return true;
               }
            }
            return false;
         }
      );
      return result;
   }
}