#include "_factory.h"
//
#include "_all.h"

namespace nifDK {
   namespace {
      template<typename T> struct check_block_type_functor {
         static bool execute(const std::string& type_name, const block* out) {
            if constexpr (block_type_has_name<T>) {
               if (type_name == T::type_name) {
                  return (dynamic_cast<const T*>(out) != nullptr);
               }
            }
            return false;
         }
      };

      template<typename T> struct create_block_functor {
         static bool execute(const std::string& type_name, block*& out) {
            if constexpr (block_type_has_name<T>) {
               if (type_name == T::type_name) {
                  out = new T();
                  return true;
               }
            }
            return false;
         }
      };
   }

   extern bool block_is_of_type(block* b, const std::string& type_name) {
      if (!b)
         return false;
      return all_block_types::for_each_breakable_with_args<check_block_type_functor>(type_name, b);
   }
   extern block* create_block_of_type(const std::string& type_name) {
      block* out = nullptr;
      all_block_types::for_each_breakable_with_args<create_block_functor>(type_name, std::forward<block*&>(out));
      if (out)
         return out;
      return new block_types::unknown_block();
   }
}