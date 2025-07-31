#include "elements.h"
#include "basic_reader.h"
#include "file_loader.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <stdexcept>
#include "../../../helpers/strings.h"
#include "../../localized_strings.h"
#include "../../logging.h"
#include "../file_load_order.h"
#include "../../localization/localized_string_store.h"

namespace dovah {
   namespace tes_file_reading {
      #pragma region group
      void group::skip() {
         assert(this->owner);
         this->owner->set_position(this->end);
      }
      uint32_t group::depth() const noexcept {
         assert(this->owner);
         for (uint32_t i = 0; i < this->owner->_groups.size(); i++) {
            auto& other = this->owner->_groups[i];
            if (&other == this)
               return i;
         }
         return std::numeric_limits<uint32_t>::max();
      }
      group* group::get_parent() const noexcept {
         assert(this->owner);
         for (uint32_t i = 1; i < this->owner->_groups.size(); i++) {
            auto parent = &this->owner->_groups[i - 1];
            auto current = &this->owner->_groups[i];
            if (current == this)
               return parent;
         }
         return nullptr;
      }
      void group::to_string(std::string& output) const noexcept {
         output.clear();
         const char* desc = "?";
         switch (this->header.type) {
            case type::forms_of_type:
               desc = dovah::logging::format_signature(_byteswap_ulong(this->header.label));
               break;
            case type::world_children:
               desc = "World Children";
               break;
            case type::interior_cell_block:
               desc = "Interior Cell Block";
               break;
            case type::interior_cell_sub_block:
               desc = "Interior Cell Sub-Block";
               break;
            case type::exterior_cell_block:
               desc = "Exterior Cell Block";
               break;
            case type::exterior_cell_sub_block:
               desc = "Exterior Cell Sub-Block";
               break;
            case type::cell_children:
               desc = "Cell Children";
               break;
            case type::topic_children:
               desc = "Topic Children";
               break;
            case type::cell_persistent_children:
               desc = "Cell Persistent Children";
               break;
            case type::cell_temporary_children:
               desc = "Cell Temporary Children";
               break;
         }
         cobb::sprintf(output, "group of type %s at depth %d, from %08X to %08X", desc, this->depth(), this->pos, this->end);
      }
      #pragma endregion

      #pragma region record
      subrecord& record::get_current_subrecord() const noexcept { return this->owner._subrecord; }
      void record::unchecked_read(void* destination, uint32_t size) {
         auto source = (std::ptrdiff_t)this->data + this->offset;
         memcpy(destination, (void*)source, size);
         this->offset += size;
      }
      bool record::read(void* destination, uint32_t size) {
         if (!this->is_in_bounds(size))
            return false;
         this->unchecked_read(destination, size);
         return true;
      }
      bool record::skip(uint32_t bytes) {
         if (!this->is_in_bounds(bytes))
            return false;
         this->offset += bytes;
         return true;
      }
      void record::return_to_start() {
         this->owner._subrecord.reset();
         this->go_to_offset(0);
      }
      subrecord& record::next_subrecord() const {
         this->owner.next_subrecord();
         return this->owner.get_current_subrecord();
      }
      uint32_t record::peek_next_subrecord_type() {
         auto pos = this->owner._subrecord.end_pos();
         if (pos < this->end) {
            auto addr = (std::ptrdiff_t)this->data;
            addr += pos;
            return _byteswap_ulong(*(uint32_t*)addr);
         }
         return 0;
      }

      void record::reset() {
         this->data.clear();
         this->offset = 0;
         this->header.signature = 0;
         this->owner._subrecord.reset();
      }

      form_stub* record::lookup_form_by_id(bare_form_id_t id) const noexcept {
         auto* file = this->owner.loader;
         if (!file)
            return nullptr;
         return file->get_load_order().get_form(id, false);
      }
      #pragma endregion

      #pragma region subrecord
      void subrecord::_fix_up_form_id(uint32_t& id) const noexcept {
         auto* file = this->owner.loader;
         if (!file)
            return;
         file->get_load_order().local_formID_to_global_formID(file, id);
      }
      bool subrecord::_read_form_id(form_id_t& field) const noexcept {
         if (this->read(field.value)) {
            this->_fix_up_form_id(field.value);
            return true;
         }
         return false;
      }
      bool subrecord::_read_form_reference(form_reference_t& field) const noexcept {
         bare_form_id_t id;
         if (!this->read(id)) {
            field.stub = nullptr;
            return false;
         }
         if (id) {
            this->_fix_up_form_id(id);
            field.stub = this->lookup_form_by_id(id);
            if (!field.stub) {
               //
               // TODO: subrecord refers to a non-existent form; generate a warning.
               //
            }
         } else
            field.stub = nullptr;
         return true;
      }
      void subrecord::_unchecked_read_form_id(form_id_t& field) const noexcept {
         this->unchecked_read(field.value);
         this->_fix_up_form_id(field.value);
      }
      void subrecord::_unchecked_read_form_reference(form_reference_t& field) const noexcept {
         bare_form_id_t id;
         this->unchecked_read(id);
         if (id) {
            this->_fix_up_form_id(id);
            field.stub = this->lookup_form_by_id(id);
            if (!field.stub) {
               //
               // TODO: subrecord refers to a non-existent form; generate a warning.
               //
            }
         } else
            field.stub = nullptr;
      }
      //
      record& subrecord::get_containing_record() const {
         return this->owner._record;
      }

      bool subrecord::read(std::string& field) const noexcept {
         field.clear();
         auto length = this->size();
         field.resize(length);
         if (length == 0)
            return true;
         if (!this->read(const_cast<char*>(field.data()), length))
            return false;
         if (field[length - 1] == '\0') // C++ std::strings + direct reading + null terminators = horrible, horrible mess
            field.resize(length - 1);
         return true;
      }
      bool subrecord::read(localized_string& field) const noexcept {
         field.value.clear();
         if (this->owner.uses_string_table()) {
            field.localized = localization_language::unknown;
            bool result = this->read(field.index);
            //
            field.value = "<LOAD FAILED>";
            if (auto* base = this->owner.loader) {
               if (auto* store = base->localization_data) {
                  field.value     = store->lookup(field.type, field.index);
                  field.localized = store->get_default_language_enum();
               }
            }
            //
            field.exists = true;
            return result;
         }
         return this->read(field.value);
      }

      bool subrecord::read_signature(uint32_t& out) const noexcept {
         if (!this->read(out))
            return false;
         out = _byteswap_ulong(out);
         return true;
      }
      bool subrecord::read_wstring(std::string& field) {
         field.clear();
         uint16_t length;
         if (this->read(length)) {
            field.resize(length);
            return this->read(const_cast<char*>(field.data()), length);
         }
         return false;
      }
      bool subrecord::read_wstring(std::wstring& field) {
         field.clear();
         uint16_t length;
         if (this->read(length)) {
            field.resize(length);
            //
            void* buffer = (void*)field.data();
            return this->read(buffer, length);
         }
         return false;
      }
      
      form_stub* subrecord::lookup_form_by_id(bare_form_id_t id) const noexcept {
         auto* file = this->owner.loader;
         if (!file)
            return nullptr;
         return file->get_load_order().get_form(id, false);
      }
      bool subrecord::form_id_can_survive_redundant_fixup(bare_form_id_t id) const noexcept {
         bare_form_id_t copy = id;
         this->_fix_up_form_id(copy);
         return copy == id;
      }
      #pragma endregion
   }
}