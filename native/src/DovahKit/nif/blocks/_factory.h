#pragma once
#include <string>

namespace nifDK {
   class block;

   // Check if the block is of the given type (or any known(!) subclass). Relies 
   // on string matching to find the type; then a dynamic cast to test the block.
   extern bool block_is_of_type(const block*, const std::string& type_name);

   extern block* create_block_of_type(const std::string& type_name);

   extern const char* get_block_typename(const block&);
}