#include "./set_name.h"
#include <string>
#include "../max_name_length.h"

#if _MSC_VER
   #include <windows.h>
   using SetThreadDescription_t = HRESULT(WINAPI*)(HANDLE, PCWSTR);

   constexpr const DWORD MS_VC_EXCEPTION = 0x406D1388;

   #pragma pack(push, 8)
   typedef struct tagTHREADNAME_INFO {
       DWORD  dwType;     // Must be 0x1000.  
       LPCSTR szName;     // Pointer to name (in user addr space).  
       DWORD  dwThreadID; // Thread ID (-1 = caller thread).  
       DWORD  dwFlags;    // Reserved for future use, must be zero.  
   } THREADNAME_INFO;
   #pragma pack(pop)

   using native_result = HRESULT;
   //
   constexpr const bool native_results_are_posix = false;
   //
   constexpr const native_result native_success_code = S_OK;
   constexpr const native_result generic_failure     = E_FAIL;
#elif __GLIBC__ > 2 && _GLIBC_MINOR__ > 6
   #include <linux/prctl.h> // PR_...
   #include <sys/prctl.h> // prctl

   using native_result = int;
   //
   constexpr const bool native_results_are_posix = true;
   //
   constexpr const native_result native_success_code = 0;
#else
   using native_result = int;
   constexpr const native_result native_success_code = 0;
#endif

namespace lu::threads::current {
   #if _MSC_VER
      //
      // This has to be its own function because SEH can't be used inside of a function 
      // that has any objects that require unwinding (i.e. anything with a destructor).
      //
      static void _set_name_internal(const char* name) {
         THREADNAME_INFO info{
            .dwType     = 0x1000,
            .szName     = name,
            .dwThreadID = (DWORD)-1,
            .dwFlags    = 0,
         };
         #pragma warning(push)
         #pragma warning(disable: 6320 6322)
         __try {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
         } __except (EXCEPTION_EXECUTE_HANDLER) {
         }
         #pragma warning(pop)
      }
   #endif


   static native_result _set_name(zstring_view name) {
      #if _MSC_VER
         static auto SetThreadDescription = (SetThreadDescription_t) GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"),
            "SetThreadDescription"
         );

         native_result result = native_success_code;
         if (SetThreadDescription) {
            std::wstring wide_name;
            wide_name.resize(name.size());
            MultiByteToWideChar(
               CP_ACP,
               0,
               name.data(),
               name.size() * sizeof(char), // size in bytes
               wide_name.data(),
               wide_name.size() + 1 // size in chars, including trailing null
            );
            result = SetThreadDescription(GetCurrentThread(), wide_name.c_str());
         }
         //
         // Also try the pre-Win10 API, which relies on us basically broadcasting the desired thread 
         // name to any debugger that happens to be listening. (If a debugger isn't listening right 
         // now, though, then don't bother.)
         //
         if (IsDebuggerPresent()) {
            _set_name_internal(name.c_str());
         }
         return result;
      #elif __GLIBC__ > 2 && _GLIBC_MINOR__ > 6
         return prctl(PR_SET_NAME, name.c_str());
      #else
         return native_success_code;
      #endif
   }

   void set_name(zstring_view name) {
      _set_name(name);
   }

   void set_name(zstring_view name, std::error_code& code) {
      auto result = _set_name(name);
      if (result == native_success_code)
         code = {};
      else
         code = std::error_code(result, native_results_are_posix ? std::generic_category() : std::system_category());
   }
}
