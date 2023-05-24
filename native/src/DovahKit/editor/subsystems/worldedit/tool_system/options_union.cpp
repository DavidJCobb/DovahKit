#include "./options_union.h"
#include <array>
#include "helpers/streams/bitreader.h"
#include "helpers/streams/bitwriter.h"

namespace {
   namespace worldedit {
      using namespace ::dovahkit::subsystems::worldedit;
   }
}

namespace dovahkit::subsystems::worldedit::tools {
   void options_union::_destroy_data() {
      if (this->tag == id_of_none)
         return;
      for (const auto& entry : _type_table) {
         if (entry.id == this->tag) {
            entry.destruct(*this);
            break;
         }
      }
   }

   /*static*/ options_union options_union::construct_for_type(tool_id id) {
      if (id == id_of_none)
         return {};
      for (const auto& entry : _type_table) {
         if (entry.id == id) {
            options_union out;
            out.tag = id;
            entry.construct(out);
            return out;
         }
      }
      return {};
   }

   options_union* options_union::clone() const {
      auto* copy = new options_union;
      copy->tag = this->tag;
      if (this->tag != id_of_none) {
         for (const auto& entry : _type_table) {
            if (entry.id != this->tag)
               continue;
            entry.construct_copy(*this, *copy);
            break;
         }
      }
      return copy;
   }

   namespace {
      template<tool_with_options_member_type T> static void _typed_read(options_union& ou, cobb::streams::bitreader& stream) {
         tools::_base::options_serialization_version version;
         stream.read(version);
         ou.as<T>().read(version, stream);
      }
      template<tool_with_options_member_type T> static void _typed_write(const options_union& ou, cobb::streams::bitwriter& stream) {
         stream.write((tools::_base::options_serialization_version)T::options::serialization_version);
         ou.as<T>().write(stream);
      }

      struct _serialization_handler_table_entry {
         tool_id id = id_of_none;
         cobb::function_pointer<void(options_union&, cobb::streams::bitreader&)> read = nullptr;
         cobb::function_pointer<void(const options_union&, cobb::streams::bitwriter&)> write = nullptr;
      };
      static constexpr const auto _serialization_handler_table = []() {
         std::array<_serialization_handler_table_entry, all_tools_with_options::count> entries = {};
         {
            size_t i = 0;
            all_tools_with_options::for_each([&entries, &i]<typename Tool>() {
               auto& entry = entries[i];
               entry.id    = id_of<Tool>;
               entry.read  = &_typed_read<Tool>;
               entry.write = &_typed_write<Tool>;
               ++i;
            });
         }
         return entries;
      }();
   }
   //
   void options_union::read(cobb::streams::bitreader& stream) {
      bool presence;
      stream.read(presence);
      if (!presence) {
         this->tag = id_of_none;
         return;
      }
      for (const auto& entry : _serialization_handler_table) {
         if (entry.id != this->tag)
            continue;
         (entry.read)(*this, stream);
         return;
      }
   }
   void options_union::write(cobb::streams::bitwriter& stream) const {
      for (const auto& entry : _serialization_handler_table) {
         if (entry.id != this->tag)
            continue;

         stream.write(true); // presence bool
         //
         (entry.write)(*this, stream);
         return;
      }
      stream.write(false); // presence bool
      return;
   }
}