#include "./property.h"
#include <cassert>
#include <stdexcept>
#include <type_traits>
#include "helpers/type_traits/is_std_vector.h"
#include "../../_common_cpp.h"
#include "../../../logging.h"
#include "../../../notice_code_list.h"
#include "./attachment_header.h"

namespace {
   template<typename T>
   bool _read_single_value(
      const dovah::loaded_forms::components::papyrus::attachment_header& header,
      dovah::tes_subrecord_reader& subrecord,
      T& dst
   ) {
      using namespace dovah::loaded_forms::components::papyrus;

      if constexpr (std::is_same_v<T, property_object_value>) {
         return dst.load(header, subrecord);
      } else if constexpr (std::is_same_v<T, std::string>) {
         return subrecord.read_length_prefixed_string<2>(dst);
      } else {
         static_assert(!std::is_same_v<T, bool> || sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
         return subrecord.read(dst);
      }
   }

   template<typename T>
   void _save_single_value(
      const dovah::loaded_forms::components::papyrus::attachment_header& header,
      dovah::tes_subrecord_writer& subrecord,
      const T& src
   ) {
      using namespace dovah::loaded_forms::components::papyrus;

      if constexpr (std::is_same_v<T, property_object_value>) {
         src.save(header, subrecord);
      } else if constexpr (std::is_same_v<T, std::string>) {
         subrecord.write_length_prefixed_string<2>(src);
      } else {
         static_assert(!std::is_same_v<T, bool> || sizeof(bool) == sizeof(uint8_t), "Bools aren't one byte on your platform. They are in the VMAD data, so rewrite this code accordingly.");
         subrecord.write(src);
      }
   }
}

namespace dovah::loaded_forms::components::papyrus {
   bool property::load(const attachment_header& header, tes_subrecord_reader& subrecord) {
      subrecord.read_length_prefixed_string<2>(this->name);
      if (!subrecord.is_in_bounds(sizeof(property_type) + sizeof(this->status)))
         return false;
      
      property_type typecode;
      subrecord.unchecked_read(typecode);
      if (header.version >= 4) {
         subrecord.unchecked_read(this->status);

         if (this->status == property_status::inherited_and_removed && typecode != property_type::none) {
            //
            // We don't necessarily need to warn for this, but it *is* invalid: the value 
            // will be discarded at run-time.
            //
         }
      }

      bool success = true;
      if (typecode != property_type::none) {
         try {
            this->value = property_value_from_type(typecode);
         } catch (std::runtime_error& e) {
            // TODO: Raise a load error indicating the bad property type.
            return false;
         }
         success = std::visit(
            [this, &header, &subrecord](auto& dst) -> bool {
               using value_type = std::decay_t<decltype(dst)>;
               if constexpr (cobb::is_std_vector<value_type>) {
                  using item_type = typename value_type::value_type;

                  uint32_t count;
                  if (!subrecord.read(count))
                     return false;
                  dst.resize(count);
                  for (size_t i = 0; i < count; ++i) {
                     if constexpr (std::is_same_v<item_type, bool>) { // std::vector<bool> was a mistake
                        uint8_t v;
                        if (!subrecord.read(v))
                           return false;
                        dst[i] = v;
                     } else {
                        if (!_read_single_value(header, subrecord, dst[i]))
                           return false;
                     }
                  }
               } else if constexpr (!std::is_same_v<value_type, std::monostate>) {
                  if (!_read_single_value(header, subrecord, dst))
                     return false;
               }
               return true;
            },
            this->value
         );
      } else {
         if (this->status != property_status::inherited_and_removed) {
            //
            // We don't necessarily need to warn for this, but it *is* invalid: the value 
            // will trigger a "type mismatch" Papyrus log warning at run-time, of the 
            // following form:
            // 
            //    error: Property pfBaseProperty on script aaaVMADTestButtonScript attached to EditorIDHere (090012D7) cannot be initialized because the value is the incorrect type
            //
            // Not sure the warning would occur if the property type on the actual PEX 
            // script were Object/Form, though, as in that case we'd just be binding None.
            //
         }
      }
      return success;
   }
   void property::save(const attachment_header& header, tes_subrecord_writer& subrecord, load_order_interfaces::form_save& intfc) noexcept {
      subrecord.write_length_prefixed_string<2>(this->name);
      subrecord.write(this->type());
      subrecord.write(this->status);
      if (!std::holds_alternative<std::monostate>(this->value)) {
         std::visit(
            [this, &header, &subrecord](const auto& src) {
               using value_type = std::decay_t<decltype(src)>;
               if constexpr (cobb::is_std_vector<value_type>) {
                  using item_type = typename value_type::value_type;

                  uint32_t count = src.size();
                  subrecord.write(count);

                  for (size_t i = 0; i < src.size(); ++i) {
                     if constexpr (std::is_same_v<item_type, bool>) { // std::vector<bool> was a mistake
                        subrecord.write(src[i]);
                     } else {
                        _save_single_value(header, subrecord, src[i]);
                     }
                  }
               } else if constexpr (!std::is_same_v<value_type, std::monostate>) {
                  _save_single_value(header, subrecord, src);
               }
            },
            this->value
         );
      }
   }
   void property::clone_from(const property& source, loaded_forms::Form& owner_of_clone) noexcept {
      this->clear(owner_of_clone);

      this->name   = source.name;
      this->status = source.status;

      std::visit(
         [this, &owner_of_clone](const auto& src) {
            using value_type = std::decay_t<decltype(src)>;
            
            if constexpr (cobb::is_std_vector<value_type>) {
               using item_type = typename value_type::value_type;

               this->value  = value_type{};
               auto& casted = std::get<value_type>(this->value);

               size_t size = src.size();
               casted.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  if constexpr (std::is_same_v<item_type, property_object_value>) {
                     casted[i].clone_from(src[i], owner_of_clone);
                  } else {
                     casted[i] = src[i];
                  }
               }
            } else if constexpr (std::is_same_v<value_type, property_object_value>) {
               this->value = property_object_value{};
               std::get<property_object_value>(this->value).clone_from(src, owner_of_clone);
            } else {
               this->value = src;
            }
         },
         source.value
      );
   }
   void property::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      if (auto* casted = std::get_if<property_object_value>(&this->value)) {
         casted->sever_outbound_references_to(target, my_owner);
      } else if (auto* casted = std::get_if<std::vector<property_object_value>>(&this->value)) {
         for (auto& item : *casted)
            item.sever_outbound_references_to(target, my_owner);
      }
   }
   void property::clear(loaded_forms::Form& my_owner) noexcept {
      this->name.clear();
      this->status = {};

      if (auto* casted = std::get_if<property_object_value>(&this->value)) {
         casted->clear(my_owner);
      } else if (auto* casted = std::get_if<std::vector<property_object_value>>(&this->value)) {
         for (auto& item : *casted)
            item.clear(my_owner);
      }
      this->value = {}; // to std::monostate
   }

   void property::set_type(loaded_forms::Form& my_owner, property_type pt) {
      if (this->type() == pt)
         return;
      this->clear(my_owner);
      this->value = property_value_from_type(pt);
   }

   /*static*/ void property::extract_name_and_skip_remainder(const attachment_header& header, tes_subrecord_reader& subrecord, std::string& out) {
      subrecord.read_length_prefixed_string<2>(out);
      skip_use_info(subrecord, true);
   }
   /*static*/ void property::generate_use_info(const attachment_header& header, tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib, bool already_read_name) {
      form_id_t formID;
      //
      if (!already_read_name) {
         subrecord.skip_length_prefixed_string<2>();
      }
      //
      property_type type;
      subrecord.read(type);
      subrecord.skip_bytes(1); // property status
      uint32_t value_count = 1;
      if (property_type_is_array(type)) {
         if (!subrecord.read(value_count))
            return;
      }
      switch (type) {
         case property_type::object:
            if (header.object_format == 2) {
               subrecord.skip_bytes(4);
               subrecord.read(formID);
               uib.add_outbound_reference(formID);
            } else {
               subrecord.read(formID);
               uib.add_outbound_reference(formID);
               subrecord.skip_bytes(4);
            }
            break;
         case property_type::string:
            subrecord.skip_length_prefixed_string<2>();
            break;
         case property_type::integer:
         case property_type::float32:
            subrecord.skip_bytes(4);
            break;
         case property_type::boolean:
            subrecord.skip_bytes(1);
            break;
         case property_type::array_of_object:
            for (uint32_t k = 0; k < value_count; k++) {
               if (header.object_format == 2) {
                  subrecord.skip_bytes(4);
                  subrecord.read(formID);
                  uib.add_outbound_reference(formID);
               } else {
                  subrecord.read(formID);
                  uib.add_outbound_reference(formID);
                  subrecord.skip_bytes(4);
               }
            }
            break;
         case property_type::array_of_string:
            for (uint32_t k = 0; k < value_count; k++)
               subrecord.skip_length_prefixed_string<2>();
            break;
         case property_type::array_of_integer:
         case property_type::array_of_float32:
            subrecord.skip_bytes(4 * value_count);
            break;
         case property_type::array_of_boolean:
            subrecord.skip_bytes(value_count);
            break;
      }
   }
   /*static*/ void property::skip_use_info(tes_subrecord_reader& subrecord, bool already_read_name) {
      form_id_t formID;
      //
      if (!already_read_name) {
         subrecord.skip_length_prefixed_string<2>();
      }
      //
      property_type type;
      subrecord.read(type);
      subrecord.skip_bytes(1); // property status
      uint32_t value_count = 1;
      if (property_type_is_array(type)) {
         if (!subrecord.read(value_count))
            return;
      }
      switch (type) {
         case property_type::object:
            subrecord.skip_bytes(8);
            break;
         case property_type::string:
            subrecord.skip_length_prefixed_string<2>();
            break;
         case property_type::integer:
         case property_type::float32:
            subrecord.skip_bytes(4);
            break;
         case property_type::boolean:
            subrecord.skip_bytes(1);
            break;
         case property_type::array_of_object:
            subrecord.skip_bytes(8 * value_count);
            break;
         case property_type::array_of_string:
            for (uint32_t k = 0; k < value_count; k++)
               subrecord.skip_length_prefixed_string<2>();
            break;
         case property_type::array_of_integer:
         case property_type::array_of_float32:
            subrecord.skip_bytes(4 * value_count);
            break;
         case property_type::array_of_boolean:
            subrecord.skip_bytes(value_count);
            break;
      }
   }
}