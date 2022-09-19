#include "manifest_parser.h"
#include <array>
#include <cstdint>
#include "../../helpers/qt/minimize_indent.h"

namespace {
   // The "permissions" feature for Dovahscript is dummied out and will remain so until I 
   // think of an actual good design for it, in terms of what the user sees and interacts 
   // with.
   static constexpr bool parse_package_permissions = false;
}

namespace {
   QString case_insensitive_attr(QXmlStreamReader& xml, QLatin1String name) {
      for (auto& attr : xml.attributes())
         if (attr.name().compare(name, Qt::CaseInsensitive) == 0)
            return attr.value().toString();
      return QString();
   }
}

namespace script_packages {
   manifest_parser::manifest_parser() {
      this->xml.setEntityResolver(&this->entity_resolver);
   }

   /*static*/ manifest::version manifest_parser::_parse_version(QStringRef text) {
      manifest::version out;
      //
      // Permitted strings:
      // 
      //    "1"              -> { 1, 0, 0, 0 }
      //    "1.2"            -> { 1, 2, 0, 0 }
      //    " 1.2.3.4"       -> { 1, 2, 3, 4 }
      //    "1 . 2 . 3 . 4 " -> { 1, 2, 3, 4 }
      // 
      // Non-permitted strings:
      // 
      //    "1.2."              // trailing dot
      //    ".2.3.4"            // leading dot
      //    "1 1 1.222.333.444" // spaces other than padding
      //    "1.2a"              // non-digit
      //    "1.2.3.4.5"         // too many components
      //
      text = text.trimmed();
      auto size  = text.size();
      int  part  = 0;
      bool space = false;
      bool empty = true;
      bool sep   = false;
      std::array<uint32_t, 4> parts = {};
      for (decltype(size) i = 0; i < size; ++i) {
         QChar c = text[i];
         if (c == '.') {
            if (empty)
               return manifest::version(); // invalid version number (e.g. ".2.3.4")
            ++part;
            if (part >= parts.size())
               return manifest::version(); // invalid version number (e.g. "1.2.3.4.5")
            space = false;
            empty = true;
            sep   = true;
            continue;
         }
         sep = false;
         if (c.isSpace()) {
            if (!empty)
               return manifest::version(); // invalid version number (e.g. "1 1 1.222.333.444")
            space = true;
            continue;
         }
         if (!c.isDigit())
            return manifest::version(); // invalid version number (non-numerical digits)
         empty = false;
         //
         parts[part] = parts[part] * 10 + c.digitValue();
      }
      if (sep)
         return manifest::version(); // invalid version number (trailing dot e.g. "1.2.")
      out.major = parts[0];
      out.minor = parts[1];
      out.patch = parts[2];
      out.build = parts[3];
      return out;
   }

   void manifest_parser::_parse_metadata() {
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("name"), Qt::CaseInsensitive) == 0) {
            this->out.name = cobb::qt::minimize_indent(xml.readElementText(QXmlStreamReader::IncludeChildElements));
            continue;
         }
         if (name.compare(QLatin1String("author"), Qt::CaseInsensitive) == 0) {
            this->_parse_author();
            continue;
         }
         if (name.compare(QLatin1String("description"), Qt::CaseInsensitive) == 0) {
            this->out.description = cobb::qt::minimize_indent(xml.readElementText(QXmlStreamReader::IncludeChildElements));
            continue;
         }
         if (name.compare(QLatin1String("version"), Qt::CaseInsensitive) == 0) {
            auto t = xml.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
            this->out.version_info.package = _parse_version(QStringRef(&t));
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

   void manifest_parser::_parse_environment() {
      while (xml.readNextStartElement()) {
         auto name = xml.name();
         if (name.compare(QLatin1String("min-version"), Qt::CaseInsensitive) == 0) {
            auto t = xml.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
            this->out.version_info.dovah_minimum = _parse_version(QStringRef(&t));
            continue;
         } else if (name.compare(QLatin1String("latest-version"), Qt::CaseInsensitive) == 0) {
            auto t = xml.readElementText(QXmlStreamReader::ErrorOnUnexpectedElement);
            this->out.version_info.dovah_tested = _parse_version(QStringRef(&t));
            continue;
         }
         xml.skipCurrentElement();
      }
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
         if (name.compare(QLatin1String("environment"), Qt::CaseInsensitive) == 0) {
            this->_parse_environment();
            continue;
         }
         if constexpr (parse_package_permissions) {
            if (name.compare(QLatin1String("permissions"), Qt::CaseInsensitive) == 0) {
               this->_parse_permissions();
               continue;
            }
         }
         if (name.compare(QLatin1String("files"), Qt::CaseInsensitive) == 0) {
            this->_parse_files();
            continue;
         }
         xml.skipCurrentElement();
      }
      if (xml.hasError())
         return false;
      if (!this->out.version_info.dovah_tested) {
         auto& min = this->out.version_info.dovah_minimum;
         if (min)
            this->out.version_info.dovah_tested = min;
      }
      return true;
   }
}