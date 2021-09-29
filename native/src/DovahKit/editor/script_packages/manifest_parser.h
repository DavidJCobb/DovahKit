#pragma once
#include <QXmlStreamReader>
#include "../../helpers/qt/xml_html_entity_resolver.h"
#include "manifest.h"

namespace script_packages {
   class manifest_parser {
      protected:
         QXmlStreamReader xml;
         cobb::qt::xml::XmlHtmlEntityResolver entity_resolver;
         manifest out;

         static manifest::version _parse_version(QStringRef);

         void _parse_metadata();
            void _parse_author();
         void _parse_environment();
         void _parse_permissions();
            void _set_bool_permission(const QString& name, bool);
         void _parse_files();

      public:
         manifest_parser();

         bool parse(const QString& code);

         inline manifest result() const noexcept { return this->out; }
         inline int error_code() const noexcept { return (int)this->xml.error(); }
         inline QString error_text() const noexcept { return this->xml.errorString(); }
   };
}
