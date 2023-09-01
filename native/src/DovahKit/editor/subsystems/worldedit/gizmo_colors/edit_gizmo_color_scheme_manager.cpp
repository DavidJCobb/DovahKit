#include "./edit_gizmo_color_scheme_manager.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "./hardcoded.h"

namespace dovahkit::subsystems::worldedit {
   gizmo_color_scheme_manager::gizmo_color_scheme_manager() : QObject(nullptr) {
      this->done_constructing();
   }

   std::vector<gizmo_color_scheme> gizmo_color_scheme_manager::all_hardcoded_color_schemes() const {
      const auto& src = hardcoded_gizmo_color_schemes;

      std::vector<gizmo_color_scheme> out;
      out.resize(src.size());
      for (size_t i = 0; i < out.size(); ++i)
         out[i] = src[i];
      return out;
   }
   const std::vector<gizmo_color_scheme>& gizmo_color_scheme_manager::all_user_color_schemes() const {
      return this->_user_schemes;
   }

   bool gizmo_color_scheme_manager::scheme_name_is_taken(const std::string& name) const {
      for (const auto& scheme : hardcoded_gizmo_color_schemes)
         if (scheme.name == name)
            return true;
      for (const auto& scheme : this->_user_schemes)
         if (scheme.name == name)
            return true;
      return false;
   }

   std::string gizmo_color_scheme_manager::add_new_color_scheme(const char* name, const gizmo_color_scheme& based_on) {
      std::string name_to_use = name;
      if (name_to_use.empty())
         return {}; // TODO: if we decide to throw exceptions, we should throw on this, too
      {
         // TODO: If `name` already has a parenthetical number, increment that instead 
         //       of appending to it.
         // 
         // TODO: Should we just throw an exception if the name is taken, and leave 
         //       incrementing the number as the UI's responsibility?
         //
         size_t attempts = 0;
         while (this->scheme_name_is_taken(name_to_use)) {
            name_to_use = name_to_use + " (" + std::to_string(attempts) + ")";
            if (++attempts > 100)
               return {};
         }
      }

      auto& scheme = this->_user_schemes.emplace_back();
      scheme = based_on;
      std::swap(scheme.name, name_to_use);
      return name_to_use;
   }

   std::optional<gizmo_color_scheme_manager::color_scheme_id> gizmo_color_scheme_manager::_parse_current_id(QXmlStreamReader& parser) {
      QString name;
      bool    is_hardcoded = false;
      for (const auto& attr : parser.attributes()) {
         auto an = attr.name();
         if (an == "name") {
            name = attr.value().toString();
         } else if (an == "is-hardcoded") {
            is_hardcoded = attr.value() == "true";
         }
      }
      parser.skipCurrentElement();

      if (name.isNull() || name.isEmpty())
         return {};
      return color_scheme_id{
         .name         = name.toUtf8().toStdString(),
         .is_hardcoded = is_hardcoded,
      };
   }
   void gizmo_color_scheme_manager::_parse_scheme(QXmlStreamReader& parser) {
      QString name;
      for (const auto& attr : parser.attributes()) {
         auto an = attr.name();
         if (an == "name") {
            name = attr.value().toString();
            break;
         }
      }
      if (name.isEmpty())
         return;

      auto& inserted = this->_user_schemes.emplace_back();
      inserted.name = name.toUtf8().toStdString();

      // Defaults:
      inserted.axes = {
         gizmo_color_scheme::rgb{ 255, 0, 0 },
         gizmo_color_scheme::rgb{ 0, 255, 0 },
         gizmo_color_scheme::rgb{ 0, 0, 255 }
      };
      inserted.highlight = { 255, 255, 0 };

      while (parser.readNextStartElement()) {
         if (parser.hasError())
            return;
         bool is_axis = parser.name() == "axis";
         bool is_high = parser.name() == "highlight";
         if (!is_axis && !is_high) {
            parser.skipCurrentElement();
            continue;
         }

         uint8_t which_axis = 0;
         if (is_axis) {
            QString value;
            bool    found = false;
            for (const auto& attr : parser.attributes()) {
               if (attr.name() == "which") {
                  found = true;
                  value = attr.value().toString();
                  break;
               }
            }
            if (!found) {
               parser.skipCurrentElement();
               continue;
            }

            if (value == "x")
               which_axis = 0;
            else if (value == "y")
               which_axis == 1;
            else if (value == "z")
               which_axis = 2;
            else {
               parser.skipCurrentElement();
               continue;
            }
         }

         // Read children of <axis/> and <highlight/>:
         while (parser.readNextStartElement()) {
            if (parser.hasError())
               return;
            if (parser.name() != "fill-color") {
               parser.skipCurrentElement();
               continue;
            }
            gizmo_color_scheme::rgb seen;
            if (is_axis) {
               seen = inserted.axes[which_axis];
            } else {
               seen = inserted.highlight;
            }

            constexpr const std::array<const char*, 3> component_names = { "r", "g", "b" };
            static_assert(std::tuple_size_v<decltype(component_names)> == std::tuple_size_v<decltype(gizmo_color_scheme::rgb::components)>);
            for (const auto& attr : parser.attributes()) {
               for (size_t i = 0; i < component_names.size(); ++i) {
                  if (attr.name() == component_names[i]) {
                     bool ok;
                     auto v = attr.value().toInt(&ok);
                     if (!ok)
                        continue;
                     if (v < 0)
                        v = 0;
                     else if (v > 255)
                        v = 255;
                     seen.components[i] = v;
                     break;
                  }
               }
            }
            if (is_axis) {
               inserted.axes[which_axis] = seen;
            } else {
               inserted.highlight = seen;
            }
            parser.skipCurrentElement();
         }
      }
   }
   void gizmo_color_scheme_manager::_parse_file(QXmlStreamReader& parser) {
      if (!parser.readNextStartElement()) {
         parser.raiseError(QObject::tr("no root element"));
         return;
      }
      if (parser.name() != "gizmo-scheme-options") {
         parser.raiseError(QObject::tr("unexpected root element: %1").arg(parser.name()));
         return;
      }

      this->_user_schemes.clear();
      color_scheme_id current_id;

      while (parser.readNextStartElement()) {
         if (parser.hasError())
            return;
         auto name = parser.name();
         if (name == "current") {
            auto result = this->_parse_current_id(parser);
            if (result.has_value())
               current_id = result.value();
            continue;
         }
         if (name == "user-defined-schemes") {
            while (parser.readNextStartElement()) {
               if (parser.name() == "scheme") {
                  this->_parse_scheme(parser);
                  continue;
               }
               parser.skipCurrentElement();
            }
         }
         parser.skipCurrentElement();
      }
      if (parser.hasError())
         return;

      this->set_current_color_scheme_id(current_id);
   }

   void gizmo_color_scheme_manager::reload() {
      auto path = dovahkit::subsystems::options::core::get().get_userdata_path() + "edit-gizmo-color-schemes.xml";
      auto file = QFile(path);
      file.open(QIODevice::ReadOnly);
      if (!file.isOpen()) {
         return;
      }
      QString xml = file.readAll();

      QXmlStreamReader parser;
      parser.addData(xml);
      this->_parse_file(parser);
      if (parser.hasError()) {
         #if _DEBUG
            __debugbreak();
         #endif
         return;
      }
      // Example of a valid doc:
      /*
<?xml whatever?>
<gizmo-scheme-options>
   <current
      name="Standard"
      is-hardcoded="true"
   />
   <user-defined-schemes>
      <scheme name="My Custom Scheme">
         <axis which="x">
            <fill-color r="255" g="0" b="0" />
         </axis>
         <axis which="y">
            <fill-color r="0" g="255" b="0" />
         </axis>
         <axis which="z">
            <fill-color r="0" g="0" b="255" />
         </axis>
         <highlight>
            <fill-color r="255" g="255" b="255" />
         </highlight>
      </scheme>
   </user-defined-schemes>
</gizmo-scheme-options>
      */
   }
   void gizmo_color_scheme_manager::save() {
      QString output;
      QXmlStreamWriter writer(&output);

      writer.writeStartElement("gizmo-scheme-options");
      if (!this->_current_scheme.name.empty()) {
         writer.writeEmptyElement("current");

         auto& id = this->_current_scheme;
         writer.writeAttribute("name", QString::fromUtf8(id.name.c_str(), id.name.size()));
         writer.writeAttribute("is-hardcoded", id.is_hardcoded ? "true" : "false");
      }
      if (!this->_user_schemes.empty()) {
         writer.writeStartElement("user-defined-schemes");
         for (const auto& item : this->_user_schemes) {
            writer.writeStartElement("scheme");
            writer.writeAttribute("name", QString::fromUtf8(item.name.c_str(), item.name.size()));

            constexpr const std::array<const char*, 3> axis_names = { "x", "y", "z" };
            constexpr const std::array<const char*, 3> rgb_names = { "r", "g", "b" };
            for (size_t i = 0; i < 3; ++i) {
               writer.writeStartElement("axis");
               writer.writeAttribute("which", axis_names[i]);

               writer.writeEmptyElement("fill-color");
               for (size_t c = 0; c < 3; ++c) {
                  writer.writeAttribute(rgb_names[c], QString::number(item.axes[i].components[c]));
               }
               writer.writeEndElement();
            }
            {
               writer.writeStartElement("highlight");

               writer.writeEmptyElement("fill-color");
               for (size_t c = 0; c < 3; ++c) {
                  writer.writeAttribute(rgb_names[c], QString::number(item.highlight.components[c]));
               }
            }

            writer.writeEndElement();
         }
         writer.writeEndElement();
      }
      writer.writeEndElement();

      auto path = dovahkit::subsystems::options::core::get().get_userdata_path() + "edit-gizmo-color-schemes.xml";
      auto file = QFile(path);
      file.open(QIODevice::WriteOnly);
      if (!file.isOpen()) {
         return;
      }
      file.write(output.toUtf8());
   }

   gizmo_color_scheme gizmo_color_scheme_manager::get_current_color_scheme() const {
      const auto& name = this->_current_scheme.name;
      if (!this->_current_scheme.name.empty()) {
         if (this->_current_scheme.is_hardcoded) {
            for (const auto& scheme : hardcoded_gizmo_color_schemes) {
               if (scheme.name == name)
                  return scheme;
            }
         } else {
            for (const auto& scheme : this->_user_schemes) {
               if (scheme.name == name)
                  return scheme;
            }
         }
      }
      return gizmo_color_scheme{
         .name = "Standard",
         .axes = {
            gizmo_color_scheme::rgb{ 255, 0, 0 },
            gizmo_color_scheme::rgb{ 0, 255, 0 },
            gizmo_color_scheme::rgb{ 0, 0, 255 }
         },
         .highlight = { 255, 255, 0 }
      };
   }
   gizmo_color_scheme_manager::color_scheme_id gizmo_color_scheme_manager::get_current_color_scheme_id() const {
      return this->_current_scheme;
   }
   void gizmo_color_scheme_manager::set_current_color_scheme_id(const color_scheme_id& id) {
      if (this->_current_scheme == id)
         return;
      
      // abort if no matching scheme exists
      bool found = false;
      if (id.is_hardcoded) {
         for (const auto& scheme : hardcoded_gizmo_color_schemes) {
            if (scheme.name == id.name) {
               found = true;
               break;
            }
         }
      } else {
         for (const auto& scheme : this->_user_schemes) {
            if (scheme.name == id.name) {
               found = true;
               break;
            }
         }
      }
      if (!found)
         return;

      this->_current_scheme = id;
      emit this->colorSchemeChanged(this->get_current_color_scheme());
   }
}