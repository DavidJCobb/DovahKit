#pragma once
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include "../name_of.h"
#include "./parse_step_info.h"

namespace cobb::bitstreams {
   class _bitstream_base {
      protected:
         std::vector<parse_step_info> _where_are_we;
         
         static constexpr const bool _enable_parse_step_tracing = true
            #ifdef __INTELLISENSE__
               && false
            #endif
         ;

         struct parse_step_guard {
            static constexpr const size_t no_index = std::numeric_limits<size_t>::max();

            constexpr parse_step_guard(_bitstream_base& o) : owner(&o) {}
            constexpr parse_step_guard(_bitstream_base& o, const parse_step_info& psi) : owner(&o) {
               this->index = all_entries().size();
               all_entries().push_back(psi);
            }
            constexpr ~parse_step_guard() {
               if (!this->empty()) {
                  all_entries().pop_back();
                  this->index = no_index;
               }
            }

            _bitstream_base* owner = nullptr;
            size_t           index = no_index;

            constexpr decltype(_bitstream_base::_where_are_we)& all_entries() const {
               return this->owner->_where_are_we;
            }

            constexpr parse_step_guard(parse_step_guard&& o) noexcept { *this = std::move(o); }
            constexpr parse_step_guard& operator=(parse_step_guard&& o) noexcept {
               std::swap(this->owner, o.owner);
               std::swap(this->index, o.index);
               return *this;
            }

            constexpr bool empty() const noexcept {
               return index == no_index;
            }
            constexpr parse_step_info& data() const {
               if (this->empty())
                  throw std::out_of_range{ "requesting parse_step_info from an empty parse_step_guard" };
               return all_entries()[this->index];
            }
         };

         template<typename T>
         [[nodiscard]] constexpr const parse_step_guard add_parse_step_for_typename(const parse_step_info& psi = {}) {
            auto out = parse_step_guard(*this);
            if constexpr (_enable_parse_step_tracing) {
               out.index = this->_where_are_we.size();
               //
               constexpr const auto& name = cobb::name_of_type<T>();
               this->_where_are_we.push_back(psi);
               this->_where_are_we.back().type_name = std::string(name.data(), name.size());
            }
            return out;
         }

      public:
         constexpr const std::vector<parse_step_info>& where_are_we() const noexcept { return this->_where_are_we; }
   };
}