#include <array>
#include <iostream>
#include <string>
#include <windows.h>

int main() {
   std::array<char, 256> buffer = {};

   std::string true_str = "true";
   const char* filename = ".\\test.ini";

   GetPrivateProfileStringA("category", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "category found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   /*// Empty lpAppName crashes; according to WINE source, that happens on NT4 and higher.
   GetPrivateProfileStringA("", "not-in-category", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "not-in-category found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';
   //*/

   {
      auto result = GetPrivateProfileIntA("category", "integer", 404, filename);
      std::cout << "[category]integer == " << result << '\n';
   }
   {
      auto result = GetPrivateProfileIntA("category", "integer-commented", 404, filename);
      std::cout << "[category]integer-commented == " << result << '\n';
   }
   {
      auto result = GetPrivateProfileIntA("category", "integer-with-garbage", 404, filename);
      std::cout << "[category]integer-with-garbage == " << result << '\n';
   }
   {
      auto result = GetPrivateProfileIntA("category", "integer-float", 404, filename);
      std::cout << "[category]integer-float == " << result << '\n';
   }

   GetPrivateProfileStringA("category", "string", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]string == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "quoted-string", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]quoted-string == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "single-string", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]single-string == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "ticked-string", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]ticked-string == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "mismatched-string", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]mismatched-string == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "string-commented", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]string-commented == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "quoted-string-with-padding", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]quoted-string-with-padding == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "quoted-string-with-garbage", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]quoted-string-with-garbage == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "string-multiple", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]string-multiple == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   GetPrivateProfileStringA("category", "string-backslash", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[category]string-backslash == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};

   /*// Empty lpAppName crashes; according to WINE source, that happens on NT4 and higher.
   GetPrivateProfileStringA("", "sentinel", "", buffer.data(), buffer.size() - 1, filename);
   std::cout << "[]sentinel == \"" << buffer.data();
   std::cout << "\"\n";
   buffer = {};
   //*/

   std::cout << '\n';

   GetPrivateProfileStringA("padded-category", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "padded-category found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA("---dashed-category", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "---dashed-category found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA(";commented-category", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << ";commented-category sentinel found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA(";commented-category", ";commented-sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << ";commented-category ;commented-sentinel found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA(";commented-category", "commented;sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << ";commented-category commented;sentinel found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA("category-trailing-garbage-a", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "category-trailing-garbage-a sentinel found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA("category-trailing-garbage-a", "garbage", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "category-trailing-garbage-a garbage found? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   GetPrivateProfileStringA("fallback-test-a", "sentinel", "false", buffer.data(), buffer.size() - 1, filename);
   std::cout << "fallback-test-a: sentinel bled over from zero-length category? " << (true_str == buffer.data());
   buffer = {};
   std::cout << '\n';

   std::cout << '\n';
   return 0;
}