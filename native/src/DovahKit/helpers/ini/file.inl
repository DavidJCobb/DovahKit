#pragma once
#include "./file.h"
#include "../string/replace_all.h"
#include "../string/strieq_ascii.h"
#include "./category.h"
#include "./file_line_parser.h"
#include "./value_serialization.h"

namespace cobb::ini {
   #pragma region file::_input_stream
   constexpr bool file::_input_stream<std::string>::has_more() const {
      return this->pos < this->src.size();
   }
   constexpr void file::_input_stream<std::string>::get_next_line(std::string& out) {
      out.clear();
      if (!this->has_more())
         return;
      auto i = this->src.find_first_of("\r\n", this->pos);
      if (i == std::string::npos) {
         out = this->src.substr(this->pos);
         this->pos = this->src.size();
         return;
      }
      out = this->src.substr(this->pos, i - this->pos);
      this->pos = i + 1;
      if (this->src[i] == '\r' && i + 1 < this->src.size() && this->src[i + 1] == '\n') {
         ++this->pos;
      }
   }
   #pragma endregion

   template<typename T>
   /*static*/ void file::_write_category(_output_stream<T>& dst, const category& cat) {
      if (cat.settings().empty())
         return;
      dst.put('\n'); // MSVC treats std::fstream.put('\n') as .write("\r\n", 2) automatically, without asking, so don't put('\r')
      std::string header;
      dst.put('[');
      dst << cat.name;
      dst << "]\n";
      for (const auto* item : cat.settings()) {
         dst << item->name;
         dst << '=';

         value_variant v = item->get_current_value_variant();
         dst << stringify_value(v);
         dst << '\n';
      }
   }

   constexpr void file::_on_category_instantiated(::cobb::passkey<file, category>, category& c) {
      this->_categories.push_back(&c);
   }
   constexpr void file::_on_setting_changed(::cobb::passkey<file, category>, setting& s, value_union old_value, value_union new_value) {
      for (global_setting_change_callback entry : this->_global_change_callbacks)
         (entry)(s, old_value, new_value);
   }

   constexpr category* file::category_by_name(std::string_view name) const {
      for (auto* c : this->_categories)
         if (::cobb::strieq_ascii(name, c->name))
            return c;
      return nullptr;
   }

   constexpr void file::add_global_setting_change_callback(global_setting_change_callback f) {
      this->_global_change_callbacks.push_back(f);
   }
   constexpr void file::remove_global_setting_change_callback(global_setting_change_callback f) {
      auto& list  = this->_global_change_callbacks;
      auto  first = std::find(list.begin(), list.end(), f);
      if (first != list.end())
         list.erase(first);
   }

   template<typename T>
   constexpr void file::_load(_input_stream<T> src) {
      if (!std::is_constant_evaluated()) {
         if (!src)
            return;
      }

      category* current_category = nullptr;

      std::string line;
      while (src.has_more()) {
         src.get_next_line(line);
         const auto parsed = default_file_line_parser{line}.parse_line();

         if (auto* casted = std::get_if<file_line_parse_results::category_start>(&parsed.content)) {
            const auto& data = *casted;
            current_category = this->category_by_name(std::string(data.name).c_str());
         } else if (auto* casted = std::get_if<file_line_parse_results::key_value_pair>(&parsed.content)) {
            if (current_category == nullptr)
               continue;
            setting* current_setting = current_category->setting_by_name(casted->key);
            if (current_setting == nullptr)
               continue;

            value_variant value_parsed = current_setting->get_current_value_variant();
            if (!std::holds_alternative<std::string>(value_parsed)) {
               //
               // Expected type is not a string.
               //
               if (casted->value_delim)
                  continue;
            }
            if (parse_value(casted->value_raw, value_parsed))
               current_setting->set_current_value_variant(value_parsed);
         }
      }
   }

   template<typename T>
   constexpr void file::_save(_output_stream<T> dst) {
      for (auto* cat : this->_categories) {
         _write_category(dst, *cat);
      }
   }

   template<typename OutType, typename InType>
   constexpr void file::_save(_output_stream<OutType> dst, _input_stream<InType> src) {
      if (!std::is_constant_evaluated()) {
         if (!src)
            return this->_save(dst);
      }
      
      auto missing_categories = this->categories();
      //
      // If the INI file already exists, read its contents: we want to preserve whitespace, comments, 
      // the order of settings, and so on. This loop reads the original file and then writes them (with 
      // any needed changes to INI setting values) to the working file.
      //

      bool before_all_categories = true;

      struct {
         category*   data = nullptr;
         std::string header_line;
         std::string body;
         std::vector<const setting*> found;
      } category_state;

      auto _write_current_category_state = [&category_state, &dst]() {
         dst << category_state.header_line;
         if (auto* cat = category_state.data) {
            //
            // Write any settings that weren't in the original file.
            //
            for (auto* setting : cat->settings()) {
               if (std::find(category_state.found.begin(), category_state.found.end(), setting) != category_state.found.end())
                  continue;
               dst << setting->name;
               dst << '=';
               dst << stringify_value(setting->get_current_value_variant());
               dst << '\n';
            }
         }
         dst << category_state.body;
      };
      
      auto handle_setting_line = [this, &category_state](std::string_view setting_name, std::string_view setting_value, const std::string_view trailing) {
         setting* current_setting = category_state.data->setting_by_name(std::string(setting_name).c_str());
         if (!current_setting) {
            category_state.body.append(setting_name);
            category_state.body += '='; // TODO: this won't preserve space-padding around the equals
            category_state.body.append(setting_value);
            category_state.body.append(trailing);
            category_state.body += '\n';
            return;
         }
         category_state.body.append(setting_name);
         category_state.body += '='; // TODO: this won't preserve space-padding around the equals
         category_state.body.append(stringify_value(current_setting->get_current_value_variant()));
         category_state.body.append(trailing);
         category_state.body += '\n';
      };

      std::string line;
      while (src.has_more()) {
         src.get_next_line(line);
         const auto parsed = default_file_line_parser{ line }.parse_line();

         if (auto* casted = std::get_if<file_line_parse_results::category_start>(&parsed.content)) {
            std::string header;
            header += parsed.leading;
            header += '[';
            header += casted->leading;
            header += casted->name;
            header += casted->trailing;
            header += ']';
            header += parsed.trailing;

            _write_current_category_state();
            category_state = {
               .data = this->category_by_name(casted->name),
               .header_line = header,
            };

            {
               auto& list = missing_categories;
               for (auto it = list.begin(); it != list.end(); ++it) {
                  auto* cat = *it;
                  if (::cobb::strieq_ascii(casted->name, cat->name)) {
                     list.erase(it);
                     break;
                  }
               }
            }
         } else if (auto* casted = std::get_if<file_line_parse_results::key_value_pair>(&parsed.content)) {
            category_state.body += parsed.leading;

            setting* current_setting = category_state.data->setting_by_name(casted->key);
            if (!current_setting) {
               category_state.body.append(casted->key);
               category_state.body += casted->between;
               if (casted->value_delim)
                  category_state.body += casted->value_delim;
               category_state.body += casted->value_raw;
               if (casted->value_delim)
                  category_state.body += casted->value_delim;
            } else {
               category_state.body.append(current_setting->name);
               category_state.body += casted->between;

               char delim = casted->value_delim;
               auto value = stringify_value(current_setting->get_current_value_variant());
               if (!casted->value_delim) {
                  auto opt_delim = default_file_line_parser::delimiter_needed_for_string(value);
                  if (opt_delim.has_value())
                     delim = opt_delim.value();
               }
               if (delim) {
                  cobb::replace_all(value, delim, std::string("\\") + delim);
                  category_state.body += delim;
                  category_state.body += value;
                  category_state.body += delim;
               } else {
                  category_state.body += value;
               }
            }

            category_state.body += parsed.trailing;
            category_state.body += '\n';
         } else if (std::holds_alternative<file_line_parse_results::ill_formed_line>(parsed.content)) {
            category_state.body += ';';
            category_state.body += line;
            category_state.body += '\n';
         } else if (std::holds_alternative<file_line_parse_results::no_op_line>(parsed.content)) {
            category_state.body += line;
            category_state.body += '\n';
         }
      }
      _write_current_category_state();

      //
      // Write any setting categories that weren't present in the existing file. (If there was 
      // no existing file, then this writes all categories, creating the file from scratch.)
      //
      for (const auto* cat : missing_categories) {
         _write_category(dst, *cat);
      }
   }

   constexpr void file::load(const std::string& src) {
      return this->_load(_input_stream<std::string>{src});
   }
   constexpr void file::save(std::string& dst) {
      return this->_save(_output_stream<std::string>{dst});
   }
   constexpr void file::save(std::string& dst, const std::string& src) {
      return this->_save(_output_stream<std::string>{dst}, _input_stream<std::string>{src});
   }
}