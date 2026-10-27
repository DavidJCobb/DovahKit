#pragma once
#include <array>
#include <cstdint>
namespace dovah {
   namespace notices {
      class base_form_load_warning;
   }
   namespace tes {
      class file;
      class record_view;
      class subrecord_view;
   }
   class active_load_order;
   class form_stub;
   enum class form_type : uint8_t;
   class managed_form_use;
}

namespace dovah::load_order_interfaces {
   class form_load {
      friend class form_stub;
      protected:
         form_load(form_stub& t);

      public:
         active_load_order& active_load_order;
         form_stub&         target_stub;
         //
         const tes::file* current_file = nullptr;
         //
         bool     is_winning_record = false;
         bool     is_partial_record = false; // you could also check the record flags, but there are certain cases where the flag should be ignored, and this bool better reflects those
         uint32_t last_record_flags = 0;

      protected:
         // Is this load occurring during the cleanup phase of a file-save operation? If 
         // so, we shouldn't log anything. The main case here is during the cleanup phase 
         // of a file-save operation, wherein form data is loaded so that we can delete 
         // unsaved forms from memory. We want to avoid actually logging any warnings 
         // because those warnings may include pointers to the to-be-deleted form, which 
         // will be left dangling.
         bool is_during_file_save_cleanup = false;

      public:
         bool is_active_file() const noexcept;

         void log_load_warning(notices::base_form_load_warning&&);

      protected:
         void _warn_if_use_is_wrong_type_impl(const managed_form_use&, const tes::subrecord_view&, const form_type* types, size_t type_count);
         void _warn_if_use_is_wrong_type_impl(const managed_form_use&, const tes::subrecord_view&, size_t nth_subrecord, const form_type* types, size_t type_count);

      public:
         void warn_if_use_is_wrong_type(const managed_form_use&, form_type, const tes::subrecord_view&);
         void warn_if_use_is_wrong_type(const managed_form_use&, form_type, const tes::subrecord_view&, size_t nth_subrecord);
         //
         template<size_t Size>
         void warn_if_use_is_wrong_type(const managed_form_use& use, const std::array<form_type, Size>& types, const tes::subrecord_view& subrecord) {
            _warn_if_use_is_wrong_type_impl(use, subrecord, types.data(), types.size());
         }
         template<size_t Size>
         void warn_if_use_is_wrong_type(const managed_form_use& use, const std::array<form_type, Size>& types, const tes::subrecord_view& subrecord, size_t nth_subrecord) {
            _warn_if_use_is_wrong_type_impl(use, subrecord, nth_subrecord, types.data(), types.size());
         }

         void warn_on_unrecognized_subrecord(const tes::subrecord_view&);
   };
}