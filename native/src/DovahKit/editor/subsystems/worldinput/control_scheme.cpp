#include "./control_scheme.h"
#include <algorithm> // std::swap
#include <bit> // std::bit_width
#include <cstdint>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/type_containers/fixed_map.h"

#include "./control_scheme/all_node_headers.h"

namespace {
   enum class _serialized_node_type {
      action,
      condition,
      modifier,
   };
}

namespace dovahkit::subsystems::worldinput {
   namespace {
      using _node_type_map = cobb::type_containers::fixed_map<
         cobb::type_containers::fixed_map_entry<control_scheme_action,         _serialized_node_type::action>,
         cobb::type_containers::fixed_map_entry<control_scheme_condition_node, _serialized_node_type::condition>,
         cobb::type_containers::fixed_map_entry<control_scheme_modifier,       _serialized_node_type::modifier>
      >;
   }
}

namespace {
   static constexpr const size_t _serialized_node_type_bitcount = std::bit_width(dovahkit::subsystems::worldinput::_node_type_map::count);
}

namespace dovahkit::subsystems::worldinput {
   control_scheme::control_scheme(input_device_type d) : device_type(d) {
   }
   
   control_scheme::control_scheme(const control_scheme& o) {
      this->name        = o.name;
      this->device_type = o.device_type;

      this->top_level_nodes.reserve(o.top_level_nodes.size());
      for (auto* o_node : o.top_level_nodes)
         this->top_level_nodes.push_back(o_node->clone());
   }
   control_scheme& control_scheme::operator=(const control_scheme& o) {
      if (&o == this)
         return *this;

      this->name        = o.name;
      this->device_type = o.device_type;

      this->clear();
      this->top_level_nodes.reserve(o.top_level_nodes.size());
      for (auto* o_node : o.top_level_nodes)
         this->top_level_nodes.push_back(o_node->clone());

      return *this;
   }
   
   control_scheme::control_scheme(control_scheme&& o) {
      std::swap(this->name,            o.name);
      std::swap(this->device_type,     o.device_type);
      std::swap(this->top_level_nodes, o.top_level_nodes);
   }
   control_scheme& control_scheme::operator=(control_scheme&& o) {
      if (&o == this)
         return *this;
      std::swap(this->name,            o.name);
      std::swap(this->device_type,     o.device_type);
      std::swap(this->top_level_nodes, o.top_level_nodes);
      return *this;
   }

   control_scheme::~control_scheme() {
      this->clear();
   }

   bool control_scheme::operator==(const control_scheme& other) const {
      if (this->device_type != other.device_type)
         return false;

      if (this->name != other.name)
         return false;

      {
         size_t size = this->top_level_nodes.size();
         if (size != other.top_level_nodes.size())
            return false;
         for (size_t i = 0; i < size; ++i) {
            auto* a = this->top_level_nodes[i];
            auto* b = other.top_level_nodes[i];
            if (*a != *b)
               return false;
         }
      }

      return true;
   }

   void control_scheme::clear() {
      for (auto* n : this->top_level_nodes)
         delete n;
      this->top_level_nodes.clear();
   }

   /*static*/ control_scheme control_scheme::read(cobb::bitstreams::reader& s) {
      input_device_type device_type;
      s.stream(device_type);

      control_scheme out = control_scheme(device_type);

      s.stream<std::bit_width(max_name_length)>(out.name);

      {
         auto stream_node = [&s](this auto&& recurse) -> node* {
            const auto pos = s.get_position();
            const _serialized_node_type type = (_serialized_node_type)s.stream_bits(_serialized_node_type_bitcount);

            node* out = nullptr;
            const bool valid = _node_type_map::for_value(type, [&s, &out]<typename Data>() {
               auto* n = node::make<Data>();
               n->data.stream(s);
               out = n;
            });
            if (!valid) {
               throw cobb::bitstreams::exceptions::bad_enum_read{ pos, (std::uintmax_t)type };
            }

            if (out->can_have_children()) {
               uint32_t size;
               s.stream(size);
               for (size_t i = 0; i < size; ++i) {
                  out->append_child( *(recurse()) );
               }
            }
            return out;
         };

         uint32_t size = 0;
         s.stream(size);
         for (size_t i = 0; i < size; ++i) {
            out.top_level_nodes.push_back(stream_node());
         }
      }
      return out;
   }
   void control_scheme::write(cobb::bitstreams::writer& s) const {
      assert(s.get_bitpos() == 0);
      {
         auto& h = s.header();
         h.versions.data = serialization_data_version;
         s.stream(h);
      }

      s.stream(this->device_type);
      s.stream<std::bit_width(max_name_length)>(this->name);
      
      auto stream_node = [&s](this auto&& recurse, node& target) -> void {
         const bool any_streamed = _node_type_map::for_each_pair_until_true([&s, &target]<typename Key, _serialized_node_type Value>() {
            if (auto* casted = target.as<Key>()) {
               s.stream_bits(_serialized_node_type_bitcount, Value);
               s.stream(casted->data);
               return true;
            }
            return false;
         });
         assert(any_streamed && "Assert: the node type was recognized and the node was serialized.");
         if (target.can_have_children()) {
            uint32_t size = target.children.size();;
            s.stream(size);
            for (size_t i = 0; i < size; ++i) {
               recurse( *(target.children[i]) );
            }
         }
      };

      uint32_t size = this->top_level_nodes.size();
      s.stream(size);
      for (size_t i = 0; i < size; ++i) {
         stream_node( *(this->top_level_nodes[i]) );
      }
   }
}