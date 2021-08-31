#include "manifest_parser.h"

namespace {
   QString case_insensitive_attr(QXmlStreamReader& xml, QLatin1String name) {
      for (auto& attr : xml.attributes())
         if (attr.name().compare(name, Qt::CaseInsensitive) == 0)
            return attr.value().toString();
      return QString();
   }
}

namespace script_packages {
   void manifest_parser::_parse_metadata() {
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
            this->out.name = xml.readElementText(QXmlStreamReader::IncludeChildElements);
            continue;
         }
         if (name.compare(QLatin1String("author"), Qt::CaseInsensitive) == 0) {
            this->_parse_author();
            continue;
         }
         if (name.compare(QLatin1String("description"), Qt::CaseInsensitive) == 0) {
            this->out.description = xml.readElementText(QXmlStreamReader::IncludeChildElements);
            continue;
         }
         xml.skipCurrentElement();
      }
   }

   void manifest_parser::_parse_author() {
      script_packages::manifest::author data;
      //
      for (auto& attr : xml.attributes()) {
         auto name = attr.name();
         if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
            data.name = attr.value().toString();
            continue;
         }
      }
      //
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("link"), Qt::CaseInsensitive) == 0) {
            script_packages::manifest::link link;
            //
            for (auto& attr : xml.attributes()) {
               auto name = attr.name();
               if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
                  link.name = attr.value().toString();
                  continue;
               }
               if (name.compare(QLatin1String("url"), Qt::CaseInsensitive) == 0) {
                  link.url = attr.value().toString();
                  continue;
               }
            }
            //
            if (!link.url.isEmpty())
               data.links.push_back(std::move(link));
         }
         xml.skipCurrentElement();
      }
      //
      if (!data.name.isEmpty() || !data.links.isEmpty())
         this->out.authors.push_back(std::move(data));
   }

   void manifest_parser::_set_bool_permission(const QString& id, bool value) {
      if (id.compare(QLatin1String("ui"), Qt::CaseInsensitive) == 0) {
         this->out.permissions.ui.basic = value;
         return;
      }
   }
   void manifest_parser::_parse_permissions() {
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("request"), Qt::CaseInsensitive) == 0) {
            auto id = case_insensitive_attr(xml, QLatin1String("id"));
            this->_set_bool_permission(id, true);
         } else if (name.compare(QLatin1String("reject"), Qt::CaseInsensitive) == 0) {
            auto id = case_insensitive_attr(xml, QLatin1String("id"));
            this->_set_bool_permission(id, false);
         }
         xml.skipCurrentElement();
      }
   }

   void manifest_parser::_parse_files() {
      auto text = xml.readElementText(QXmlStreamReader::IncludeChildElements).split('\n');
      for (auto& line : text) {
         line = line.trimmed();
         for (int i = 0; i < line.size(); ++i) { // strip leading path separators
            if (line[i] != '/' && line[i] != '\\') {
               line = line.mid(i);
               break;
            }
         }
         if (line.isEmpty())
            continue;
         this->out.files.push_back(std::move(line));
      }
   }

   bool manifest_parser::parse(const QString& text) {
      this->xml.clear();
      this->xml.addData(text);
      if (!xml.readNextStartElement()) {
         xml.raiseError(QObject::tr("no root element"));
         return false;
      }
      if (xml.name().compare(QLatin1String("package"), Qt::CaseInsensitive) != 0) {
         xml.raiseError(QObject::tr("unexpected root element: %1").arg(xml.name()));
         return false;
      }
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("metadata"), Qt::CaseInsensitive) == 0) {
            this->_parse_metadata();
            continue;
         }
         if (name.compare(QLatin1String("permissions"), Qt::CaseInsensitive) == 0) {
            this->_parse_permissions();
            continue;
         }
         if (name.compare(QLatin1String("files"), Qt::CaseInsensitive) == 0) {
            this->_parse_files();
            continue;
         }
         xml.skipCurrentElement();
      }
      if (xml.hasError())
         return false;
      return true;
   }
}