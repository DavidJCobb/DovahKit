#include "./form_stub_drag_drop.h"
#include <cstdint>
#include <QByteArray>
#include <QDataStream>
#include "../core.h"

namespace editor_helpers {
   extern dovah::form_stub* single_form_stub_from_mime_data(const QMimeData& mime_data) {
      if (!mime_data.hasFormat(single_form_stub_mime_type))
         return nullptr;
      uint32_t id = 0;
      {
         QByteArray  bytes = mime_data.data(single_form_stub_mime_type);
         QDataStream stream(&bytes, QIODevice::ReadOnly);
         while (!stream.atEnd()) {
            uint8_t delim;
            stream.readRawData((char*)&id, 4);
            stream.readRawData((char*)&delim, 1);
            if (delim)
               return nullptr;
         }
      }
      if (id)
         return DovahKitCore::get().get_form(id);
      return nullptr;
   }
   extern std::vector<dovah::form_stub*> form_stubs_from_mime_data(const QMimeData& mime_data) {
      if (!mime_data.hasFormat(form_stub_array_mime_type))
         return {};

      auto& editor = DovahKitCore::get();

      std::vector<dovah::form_stub*> out;
      {
         QByteArray  bytes = mime_data.data(form_stub_array_mime_type);
         QDataStream stream(&bytes, QIODevice::ReadOnly);
         while (!stream.atEnd()) {
            uint32_t id = 0;
            uint8_t  delim;
            stream.readRawData((char*)&id, 4);
            stream.readRawData((char*)&delim, 1);
            assert(!delim);

            auto* stub = editor.get_form(id);
            if (stub)
               out.push_back(stub);
         }
      }
      return out;
   }

   extern void add_form_stubs_to_mime_data(QMimeData& mime_data, const std::vector<dovah::form_stub*>& list) {
      if (list.empty())
         return;

      QByteArray  data;
      QDataStream stream(&data, QIODevice::WriteOnly);

      for (auto* stub : list) {
         char id[5] = { 0, 0, 0, 0, 0 }; // null bytes as delimiters
         if (stub)
            *(uint32_t*)id = stub->formID;
         data.append(id, 5);
      }
      mime_data.setData(form_stub_array_mime_type, data);

      if (list.size() == 1) {
         //
         // Some widgets may only accept one form at a time. Easiest way to facilitate 
         // that is to have a separate MIME type for it.
         //
         QByteArray single_data;
         char id[5] = { 0, 0, 0, 0, 0 }; // null bytes as delimiters
         if (list[0])
            *(uint32_t*)id = list[0]->formID;
         single_data.append(id, 5);
         mime_data.setData(single_form_stub_mime_type, single_data);
      }
   }
}