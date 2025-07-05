#include "./package_data.h"
#include "../../_common_cpp.h"
#include <array>
#include <utility>
#include "./package_data/_all.h"

#include "../../../notices/form_load_warnings/by_form_type/package/package_data_header_missing.h"
#include "../../../notices/form_load_warnings/by_form_type/package/package_data_unrecognized_typename.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace {
   constexpr const auto typename_mapping = []() {
      using type = dovah::packages::package_data_type;
      using pair = std::pair<std::string_view, type>;
      return std::array{
         pair{ "Bool", type::boolean },
         pair{ "Float", type::float32 },
         pair{ "Int", type::integer },
         pair{ "Location", type::location },
         pair{ "ObjectList", type::object_list },
         pair{ "SingleRef", type::single_ref },
         pair{ "TargetSelector", type::target_selector },
         pair{ "Topic", type::topic },
      };
   }();
}

namespace dovah::loaded_forms::structs::custom_packages {
   /*static*/ std::unique_ptr<package_data> package_data::load_content(tes_record_reader& record, load_order_interfaces::form_load& intfc, const load_context& context) {
      if (auto sig = record.get_current_subrecord().signature(); sig != subrecord_typename) {
         specific_load_warnings::package_data_header_missing notice(
            intfc.target_stub,
            context.which,
            sig
         );
         intfc.log_load_warning(notice);

         record.next_subrecord();
         return std::make_unique<package_data_unknown>();
      }
      std::unique_ptr<package_data> result;
      {
         std::string serialized_typename;
         record.get_current_subrecord().read(serialized_typename);
         record.next_subrecord();

         package_data_type type = package_data_type::invalid;
         for (const auto& item : typename_mapping) {
            if (item.first == serialized_typename) {
               type = item.second;
               break;
            }
         }
         if (type == package_data_type::invalid) {
            specific_load_warnings::paackage_data_unrecognized_typename notice(
               intfc.target_stub,
               context.which,
               serialized_typename
            );
            intfc.log_load_warning(notice);

            //
            // We could soldier on if we wanted to -- try to skip to the next 
            // ANAM subrecord -- but it's worth noting that the game and CK 
            // don't. They just give up if they encounter any invalid Package 
            // Data: the rest of the form is a lost cause.
            //

            result = std::make_unique<package_data_unknown>();
            //
            auto* casted = (package_data_unknown*)result.get();
            casted->type = serialized_typename;
            return std::move(result);
         }

         switch (type) {
            case package_data_type::boolean:
               result = std::make_unique<package_data_bool>();
               break;
            case package_data_type::float32:
               result = std::make_unique<package_data_float>();
               break;
            case package_data_type::integer:
               result = std::make_unique<package_data_int>();
               break;
            case package_data_type::location:
               result = std::make_unique<package_data_location>();
               break;
            case package_data_type::object_list:
               result = std::make_unique<package_data_object_list>();
               break;
            case package_data_type::single_ref:
               result = std::make_unique<package_data_single_ref>();
               break;
            case package_data_type::target_selector:
               result = std::make_unique<package_data_target_selector>();
               break;
            case package_data_type::topic:
               result = std::make_unique<package_data_topic>();
               break;
         }
         assert(result != nullptr);
      }

      // The loader supports pulling metadata from here or from the name map.
      result->load_metadata(record, intfc);

      result->load_value(record, intfc, context);

      return std::move(result);
   }
   /*static*/ void package_data::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (record.get_current_subrecord().signature() != subrecord_typename) {
         record.next_subrecord();
         return;
      }
      std::string serialized_typename;
      record.get_current_subrecord().read(serialized_typename);
      record.next_subrecord();

      package_data_type type = package_data_type::invalid;
      for (const auto& item : typename_mapping) {
         if (item.first == serialized_typename) {
            type = item.second;
            break;
         }
      }
      if (type == package_data_type::invalid) {
         return;
      }

      // Counterparts to `load_metadata`.
      if (record.get_current_subrecord().signature() == subrecord_var_name)
         record.next_subrecord();
      if (record.get_current_subrecord().signature() == subrecord_access)
         record.next_subrecord();

      // Counterparts to `load_value`.
      switch (type) {
         case package_data_type::boolean:
            package_data_bool::generate_use_info(record, uib);
            break;
         case package_data_type::float32:
            package_data_float::generate_use_info(record, uib);
            break;
         case package_data_type::integer:
            package_data_int::generate_use_info(record, uib);
            break;
         case package_data_type::location:
            package_data_location::generate_use_info(record, uib);
            break;
         case package_data_type::object_list:
            package_data_object_list::generate_use_info(record, uib);
            break;
         case package_data_type::single_ref:
            package_data_single_ref::generate_use_info(record, uib);
            break;
         case package_data_type::target_selector:
            package_data_target_selector::generate_use_info(record, uib);
            break;
         case package_data_type::topic:
            package_data_topic::generate_use_info(record, uib);
            break;
      }
   }

   void package_data::load_metadata(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_var_name) {
            subrecord.read(this->name);
            record.next_subrecord();
         }
      }
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_access) {
            uint32_t dword = 0;
            subrecord.read(dword);
            this->is_public = dword != 0;
            record.next_subrecord();
         }
      }
   }

   void package_data::save_typename(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (auto* casted = dynamic_cast<package_data_unknown*>(this)) {
         record.write_string_subrecord(subrecord_typename, casted->type);
         return;
      }
      auto type = this->get_type();
      for (const auto& item : typename_mapping) {
         if (item.second == type) {
            record.write_string_subrecord(subrecord_typename, std::string(item.first));
            return;
         }
      }
      assert(false && "A `package_data_type` value isn't mapped to a serialized typename!");
   }
   void package_data::save_unique_id(tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) {
      subrecord.write(this->unique_id);
   }
   void package_data::save_metadata(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_string_subrecord(subrecord_var_name, this->name);
      {
         auto& subrecord = record.open_next_subrecord(subrecord_access);
         subrecord.write(this->is_public ? uint32_t(1) : uint32_t(0));
         subrecord.close();
      }
   }
}