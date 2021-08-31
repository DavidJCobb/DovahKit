#pragma once
#include <QXmlStreamReader>
#include "manifest.h"

namespace script_packages {
   class manifest_parser {
      protected:
         QXmlStreamReader xml;
         manifest out;

         void _parse_metadata();
            void _parse_author();
         void _parse_permissions();
            void _set_bool_permission(const QString& name, bool);
         void _parse_files();

      public:
         bool parse(const QString& code);

         inline manifest result() const noexcept { return this->out; }
         inline int error_code() const noexcept { return (int)this->xml.error(); }
         inline QString error_text() const noexcept { return this->xml.errorString(); }
   };
}
