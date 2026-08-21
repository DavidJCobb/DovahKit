#include "./condition_mime_data.h"
#include <QIODevice>
#include "editor/core.h"
#include "editor/helpers/stringify_conditions.h"

namespace editor_helpers {
   extern std::vector<ui::types::conditions::condition> conditions_from_mime_data(const QMimeData& mime_data) {
      if (!mime_data.hasFormat(form_condition_array_mime_type))
         return {};

      auto& editor = DovahKitCore::get();

      std::vector<ui::types::conditions::condition> out;
      {
         QByteArray  bytes = mime_data.data(form_condition_array_mime_type);
         QDataStream stream(&bytes, QIODevice::ReadOnly);
         while (!stream.atEnd()) {
            auto& item = out.emplace_back();
            pull_condition_from_mime_data_stream(stream, item);
         }
      }
      return out;

   }

   extern void append_condition_to_mime_data_stream(QDataStream& stream, const ui::types::conditions::condition& src) {
      auto _stream_form = [&stream](dovah::form_stub* stub) {
         uint32_t form_id = 0;
         if (stub)
            form_id = stub->formID;
         stream << form_id;
      };

      {  // Run On
         stream << src.run_on.type;
         bool is_index = std::holds_alternative<uint32_t>(src.run_on.entity);
         stream << is_index;
         if (is_index) {
            stream << std::get<uint32_t>(src.run_on.entity);
         } else {
            dovah::form_stub* stub = nullptr;
            if (std::holds_alternative<dovah::form_stub*>(src.run_on.entity))
               stub = std::get<dovah::form_stub*>(src.run_on.entity);
            _stream_form(stub);
         }
      }
      stream << src.function;
      stream << src.flags.or_linked;
      stream << src.flags.swap_subject_and_target;
      stream << src.override_types_with;

      bool has_event_params = src.event_parameters.has_value();
      stream << has_event_params;
      if (has_event_params) {
         auto& params = src.event_parameters.value();
         stream << params.function;
         stream << params.member;
         _stream_form(params.form);
      } else {
         for (auto& p : src.parameters) {
            stream << (uint8_t)p.index();
            if (auto* casted = std::get_if<float>(&p)) {
               stream << *casted;
            } else if (auto* casted = std::get_if<int32_t>(&p)) {
               stream << *casted;
            } else if (auto* casted = std::get_if<std::string>(&p)) {
               auto str = QString::fromUtf8(QByteArray::fromStdString(*casted));
               stream << str;
            } else if (auto* casted = std::get_if<char>(&p)) {
               stream << *casted;
            } else if (std::holds_alternative<dovah::form_stub*>(p)) {
               auto* stub = std::get<dovah::form_stub*>(p);
               _stream_form(stub);
            } else if (auto* casted = std::get_if<uint32_t>(&p)) {
               stream << *casted;
            }
         }
      }

      stream << src.comparison.op;
      bool operand_is_form = std::holds_alternative<dovah::form_stub*>(src.comparison.operand);
      stream << operand_is_form;
      if (operand_is_form) {
         auto* stub = std::get<dovah::form_stub*>(src.comparison.operand);
         _stream_form(stub);
      } else {
         stream << std::get<float>(src.comparison.operand);
      }
   }
   extern void pull_condition_from_mime_data_stream(QDataStream& stream, ui::types::conditions::condition& dst) {
      auto& editor = DovahKitCore::get();

      auto _stream_form = [&stream, &editor](dovah::form_stub*& stub) {
         uint32_t form_id = 0;
         stream >> form_id;
         stub = editor.get_form(form_id);
      };

      {  // Run On
         stream >> dst.run_on.type;
         bool is_index;
         stream >> is_index;
         if (is_index) {
            stream >> dst.run_on.entity.emplace<uint32_t>();
         } else {
            auto& stub = dst.run_on.entity.emplace<dovah::form_stub*>();
            _stream_form(stub);
         }
      }
      stream >> dst.function;
      stream >> dst.flags.or_linked;
      stream >> dst.flags.swap_subject_and_target;
      stream >> dst.override_types_with;

      bool has_event_params;
      stream >> has_event_params;
      if (has_event_params) {
         auto& params = dst.event_parameters.emplace();
         stream >> params.function;
         stream >> params.member;
         _stream_form(params.form);
      } else {
         for (auto& p : dst.parameters) {
            uint8_t index;
            stream >> index;
            switch (index) {
               using working_parameter = ui::types::conditions::parameter;
               case 0:
                  break;
               case 1:
                  static_assert(std::is_same_v< std::variant_alternative_t<1, working_parameter>, uint32_t>);
                  {
                     auto& v = p.emplace<uint32_t>();
                     stream >> v;
                  }
                  break;
               case 2:
                  static_assert(std::is_same_v< std::variant_alternative_t<2, working_parameter>, char>);
                  {
                     auto& v = p.emplace<char>();
                     stream >> v;
                  }
                  break;
               case 3:
                  static_assert(std::is_same_v< std::variant_alternative_t<3, working_parameter>, float>);
                  {
                     auto& v = p.emplace<float>();
                     stream >> v;
                  }
                  break;
               case 4:
                  static_assert(std::is_same_v< std::variant_alternative_t<4, working_parameter>, int32_t>);
                  {
                     auto& v = p.emplace<int32_t>();
                     stream >> v;
                  }
                  break;
               case 5:
                  static_assert(std::is_same_v< std::variant_alternative_t<5, working_parameter>, dovah::form_stub*>);
                  {
                     auto& v = p.emplace<dovah::form_stub*>();
                     _stream_form(v);
                  }
                  break;
               case 6:
                  static_assert(std::is_same_v< std::variant_alternative_t<6, working_parameter>, std::string>);
                  {
                     auto& v = p.emplace<std::string>();
                     QString q;
                     stream >> q;
                     v = q.toStdString();
                  }
                  break;
            }
         }
      }

      stream >> dst.comparison.op;
      bool operand_is_form;
      stream >> operand_is_form;
      if (operand_is_form) {
         _stream_form(dst.comparison.operand.emplace<dovah::form_stub*>());
      } else {
         stream >> dst.comparison.operand.emplace<float>();
      }
   }

   extern void add_conditions_to_mime_data(QMimeData& mime_data, const std::vector<ui::types::conditions::condition>& list, const ui::types::conditions::context& context) {
      {  // plaintext
         QString text;
         for (size_t i = 0; i < list.size(); ++i) {
            const auto& cnd = list[i];
            text += editor_helpers::stringify_condition(cnd, context);
            text += ' ';
            text += editor_helpers::stringify_condition_boolean_operator(cnd);
            if (i + 1 < list.size())
               text += '\n';
         }
         mime_data.setText(text);
      }
      {  // binary
         QByteArray  data;
         QDataStream stream(&data, QIODevice::WriteOnly);

         for (const auto& cnd : list)
            append_condition_to_mime_data_stream(stream, cnd);

         mime_data.setData(form_condition_array_mime_type, data);
      }
   }
}