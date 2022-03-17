#pragma once
#include <stdexcept>
#include <string>

namespace vulkanDK {
   class exception : public std::exception {
      using std::exception::exception;
   };

   class vuid_exception : public exception {
      protected:
         std::string message;
      public:
         uint32_t vuid = 0; // Valid Usage ID tag

         vuid_exception(uint32_t vu, const std::string& m = "") : vuid(vu), message(m) {}
         virtual const char* what() const noexcept { return this->message.c_str(); }
   };

   class result_exception : public exception {
      protected:
         std::string message;
      public:
         VkResult result = {};

         result_exception(VkResult r, const std::string& m = "") : result(r), message(m) {}
         virtual const char* what() const noexcept { return this->message.c_str(); }
   };
}