#pragma once
#include "../extra_data.h"

namespace dovah::loaded_forms::components {
   template<uint32_t signature, extra_data_type et, int bytecount> class buffer_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a fixed-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::array<uint8_t, bytecount> bytes;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            subrecord.read(this->bytes.data(), this->bytes.size());
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->bytes.data());
            subrecord.close();
         }
         static void generate_use_info(tes_record_reader&, form_stub*) {}
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new buffer_extra_data<signature, et, bytecount>();
            for (int i = 0; i < bytecount; ++i)
               clone->bytes[i] = this->bytes[i];
            return clone;
         }
   };
   template<uint32_t signature, extra_data_type et> class binary_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a variable-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::vector<uint8_t> bytes;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            auto size = subrecord.size();
            this->bytes.resize(size);
            subrecord.read(this->bytes.data(), size);
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->bytes.data(), this->bytes.size());
            subrecord.close();
         }
         static void generate_use_info(tes_record_reader&, form_stub*) {}
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new binary_extra_data<signature, et>();
            size_t size = this->bytes.size();
            clone->bytes.resize(size);
            for (size_t i = 0; i < size; ++i)
               clone->bytes[i] = this->bytes[i];
            return clone;
         }
   };
   template<uint32_t signature, extra_data_type et> class empty_extra_data : public basic_extra_data {
      //
      // Use for when the game actively ignores the content of a subrecord, treating its mere 
      // presence as cause to create or modify some data at run-time.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.close();
         }
         static void generate_use_info(tes_record_reader&, form_stub*) {}
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new empty_extra_data<signature, et>();
            return clone;
         }
   };
   template<uint32_t signature, extra_data_type et> class float_extra_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = signature;
         //
         float value = 0.0F;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() == signature) {
               subrecord.read(this->value);
               return load_result::succeeded;
            }
            return load_result::unrecognized;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->value);
            subrecord.close();
         }
         static void generate_use_info(tes_record_reader&, form_stub*) {}
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new float_extra_data<signature, et>();
            clone->value = this->value;
            return clone;
         }
   };
   template<uint32_t signature, extra_data_type et> class formID_extra_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = signature;
         //
         form_id_t formID;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() == signature) {
               subrecord.read(this->formID);
               return load_result::succeeded;
            }
            return load_result::unrecognized;
         }
         virtual void save(tes_record_writer& record) override {
            record.write_formID_subrecord(signature, this->formID);
         }
         static void generate_use_info(tes_record_reader& record, form_stub* stub) {
            auto&     subrecord = record.get_current_subrecord();
            form_id_t formID;
            subrecord.read(formID);
            if (formID)
               stub->add_outbound_reference(formID);
         }
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new formID_extra_data<signature, et>();
            clone->formID.set(&clone_owner, this->formID);
            return clone;
         }
         virtual void clear_contained_formIDs(form_stub& my_owner) override {
            this->formID.set(&my_owner, bare_form_id_t(0));
         }
         virtual void sever_outbound_references_to(form_stub& target, form_stub& my_owner) override {
            if (this->formID == target.formID)
               this->formID.set(&my_owner, bare_form_id_t(0));
         }
   };
   template<uint32_t signature, extra_data_type et> class string_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a variable-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::string value;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual load_result load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            auto size = subrecord.size();
            this->value.resize(size);
            subrecord.read(this->value.data(), size);
            auto length = this->value.find_last_not_of('\0');
            if (length != std::string::npos && length != size)
               this->value.resize(length + 1);
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->value.data(), this->value.size() + 1);
            subrecord.close();
         }
         static void generate_use_info(tes_record_reader&, form_stub*) {}
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override {
            auto* clone = new string_extra_data<signature, et>();
            clone->value = this->value;
            return clone;
         }
   };
}