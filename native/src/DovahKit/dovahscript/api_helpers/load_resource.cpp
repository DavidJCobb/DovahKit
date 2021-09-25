#include "load_resource.h"
#include <array>
#include <filesystem>
#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QImage>
#include <QImageReader>
#include "../../helpers/lua/error.h"
#include "../../helpers/endian.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include "../../editor/core.h"

#include "../core/subsystems/coordinator.h"
#include "../core/subsystems/resources.h"
#include "../core/subsystems/resources/DovahscriptResource.h"
#include "../tasks/s2m/lambda.h"
#include "../push_native_object.h"
#include "../send_script_task.h"

//
// Planned type filters:
// 
//  - general classes
//     - audio
//     - binary
//     - image
//        = Remember that not all images are rasters; SVG, for example, wouldn't be a raster format.
//     - raster
//     - text
// 
//  - formats
//     - bmp (image, raster)
//     - dds (image, raster)
//     - gif (image, raster)
//     - jpg (image, raster) // "jpeg" aliases to this
//     - png (image, raster)
//     - txt (text)
//

namespace {
   QString _argument_field_as_string(lua_State* L, int arg_index, const char* name, bool allow_nil) {
      lua_getfield(L, arg_index, name);
      if (allow_nil && lua_isnoneornil(L, -1)) {
         lua_pop(L, 1);
         return QString();
      }
      size_t      size = 0;
      const char* text = nullptr;
      QString     out;
      //
      text = luaL_checklstring(L, -1, &size); // allows us to handle nulls inside the string
      if (!text) {
         if (luaL_callmeta(L, -1, "__tostring")) {
            text = luaL_checklstring(L, -1, &size);
            lua_remove(L, -2);
         }
      }
      if (text) {
         out = QString::fromUtf8(text, size);
      }
      lua_pop(L, 1);
      if (!text) {
         if (allow_nil)
            cobb::lua::error(L, "argument.%s was neither nil, a string, nor convertible to a string", name);
         cobb::lua::error(L, "argument.%s was not a string and was not convertible to a string", name);
      }
      return out;
   }
}

namespace {
   enum class _category {
      audio,
      binary,
      image,
      raster,
      text,
   };

   constexpr std::array _formats = {
      std::pair{ _category::audio,  "audio"},
      std::pair{ _category::binary, "binary"},
      std::pair{ _category::image,  "image"},
      std::pair{ _category::raster, "raster"},
      std::pair{ _category::text,   "text"},
      //
      std::pair{ _category::binary, "bin"},
      //
      std::pair{ _category::raster, "bmp"},
      std::pair{ _category::raster, "dds"},
      std::pair{ _category::raster, "gif"},
      std::pair{ _category::raster, "jpg"},
      std::pair{ _category::raster, "jpeg"},
      std::pair{ _category::raster, "png"},
      //
      std::pair{ _category::text, "txt"},
      std::pair{ _category::text, "utf-8"},
      std::pair{ _category::text, "utf-16"},
   };

   // Quick-and-dirty tests to rapidly check whether a file *is likely to be* valid. These aren't 
   // hard validity checks; they're present to rapidly rule out invalid files before proceeding 
   // into something more rigorous like QImageReader.
   namespace simple_format_tests {
      bool bmp(const QByteArray& buffer) {
         auto* raw  = (const uint8_t*)buffer.constData();
         int   size = buffer.size();
         if (size < 0x1A)
            return false; // invalid: too small
         if (raw[0] != 'B' || raw[1] != 'M')
            return false; // invalid: no sentinel
         //
         // First few bytes are BITMAPFILEHEADER. After that, a variable header -- typically a 
         // BITMAPCOREHEADER or the extended BITMAPINFOHEADER.
         //
         uint32_t file_size = cobb::endian_cast<std::endian::little>(*(const uint32_t*)(raw + 0x02));
         uint32_t pixels_at = cobb::endian_cast<std::endian::little>(*(const uint32_t*)(raw + 0x0A));
         uint32_t hdr_size  = cobb::endian_cast<std::endian::little>(*(const uint32_t*)(raw + 0x0E));
         if (file_size > size)
            return false; // invalid: header specified an invalid size (file truncated?)
         //
         // We can retrieve the width, height, and bits per pixel, but contrary to popular belief, 
         // modern BMPs actually can use compression, so we can't check anything useful that way.
         //
         return true;
      }
      bool jpg(const QByteArray& buffer) {
         //
         // JPEG doesn't use a header, so the only real way to validate JPEG data is to skim through 
         // it.
         //
         auto* raw  = (const uint8_t*)buffer.data();
         auto  size = buffer.size();
         bool  in_entropy = false;
         if (!size)
            return false;
         for (int i = 0; i < size; ++i) {
            if (raw[i] != 0xFF) {
               if (!in_entropy)
                  return false; // invalid: unexpected byte
               continue;
            }
            //
            // 0xFF indicates a header when not in entropy-encoded data. When in entropy-encoded data, 
            // every 0xFF byte will be followed by a superfluous 0 byte if it's data; otherwise, it's 
            // the start of a new section.
            //
            if (i + 1 >= size)
               return false; // invalid: untyped signature, or entropy 0xFF byte with no null suffix
            auto next = raw[i + 1];
            if (in_entropy) {
               if (next == 0)
                  continue;
               if (next == 0xFF) // after entropy-encoded data, 0xFF can also be used as padding before a marker
                  continue;
               in_entropy = false;
            }
            bool has_length  = false;
            bool has_entropy = false;
            if (next >= 0xE0 && next <= 0xE9) {
               has_length = true;
            } else if (next >= 0xD0 && next <= 0xD7) {
               // Restart Marker
            } else {
               switch (next) {
                  case 0xD8: // Start of Image
                  case 0xD9: // End of Image
                     break;
                  case 0xC0: // Start of Frame (baseline DCT)
                  case 0xC2: // Start of Frame (progressive DCT)
                  case 0xC4: // Define Huffman Table(s)
                  case 0xDB: // Define Quantization Table(s)
                  case 0xFE: // Comment
                     has_length = true;
                     break;
                  case 0xDA: // Start of Scan
                     has_length  = true;
                     has_entropy = true;
                     break;
                  case 0xDD: // Define Restart Interval (always four bytes of data; special-cased below)
                     break;
                  default:
                     return false; // invalid: unknown signature
               }
            }
            //
            int skip = 2; // size of the header
            if (has_length) {
               if (i + 3 >= size)
                  return false; // invalid: no room for the expected two-byte length value
               int length = cobb::endian_cast<std::endian::big>(*(uint16_t*)(&raw[i + 2]));
               length -= 2; // length includes itself
               if (length < 0)
                  return false; // invalid
               //
               skip += 2; // skip the size of the length
               skip += length;
            } else if (next == 0xDD) { // Define Restart Interval
               skip += 4;
            }
            i += skip - 1; // the for-loop will also increment
            //
            if (has_entropy) {
               in_entropy = true;
            }
         }
         return true;
      }
      bool png(const QByteArray& buffer) {
         constexpr std::array<uint8_t, 8> bytes = { 0x89, 'P', 'N', 'G', 0xD, 0xA, 0x1A, 0xA };
         if (buffer.size() <= bytes.size())
            return false;
         //
         auto* raw = (const uint8_t*)buffer.data();
         for (size_t i = 0; i < bytes.size(); ++i)
            if (raw[i] != bytes[i])
               return false;
         //
         int i = bytes.size();
         if (i + 8 >= buffer.size()) // no chunks!
            return false;
         bool found_ihdr = false;
         bool found_plte = false;
         bool found_idat = false;
         bool found_iend = false;
         bool allow_idat = false;
         while (i + 8 < buffer.size()) {
            if (found_iend) {
               return false; // invalid: chunks after IEND
            }
            uint32_t length = cobb::endian_cast<std::endian::big>(*(const uint32_t*)(raw + i));
            //
            // Validate chunk name: must be four ASCII letters in sequence:
            //
            for (int j = 0; j < 4; ++j) {
               uint8_t byte = raw[i + 4 + j];
               if (byte < 'A' || byte > 'z' || (byte > 'Z' && byte < 'a'))
                  return false;
            }
            if (raw[i + 4] == 'I' || raw[i + 4] == 'P') { // may be a critical chunk
               uint32_t signature = cobb::endian_cast<std::endian::big>(*(const uint32_t*)(raw + i + 4));
               //
               if (!found_ihdr && signature != 'IHDR')
                  return false; // invalid: missing or late IHDR
               //
               switch (signature) {
                  case 'IHDR':
                     if (found_ihdr)
                        return false; // invalid: multiple IHDR
                     found_ihdr = true;
                     allow_idat = true;
                     break;
                  case 'PLTE':
                     if (found_idat)
                        return false; // invalid: PLTE after IDAT
                     if (found_plte)
                        return false; // invalid: multiple PLTE
                     found_plte = true;
                     break;
                  case 'IDAT':
                     if (!allow_idat)
                        return false; // invalid: non-consecutive IDAT
                     found_idat = true;
                     break;
                  case 'IEND':
                     found_iend = true;
                     break;
               }
            } else {
               if (!found_ihdr)
                  return false; // invalid: missing or late IHDR
               if (found_idat)
                  allow_idat = false; // do not allow non-consecutive IDAT
            }
            //
            i += 8; // signature and length
            i += length;
            i += 4; // skip the CRC
         }
         return true;
      }
   }
}

namespace dovahscript::api_helpers {
   extern load_resource_params pull_load_resource_params(lua_State* L, int arg_index) {
      assert(lua_absindex(L, arg_index) == arg_index);
      //
      load_resource_params out;
      switch (lua_type(L, arg_index)) {
         case LUA_TSTRING:
            out.path = QString::fromUtf8(lua_tostring(L, arg_index));
            break;
         case LUA_TTABLE:
            [[fallthrough]];
         case LUA_TUSERDATA:
            out.path = _argument_field_as_string(L, arg_index, "path", false);
            out.type = _argument_field_as_string(L, arg_index, "type", true).trimmed();
            break;
         default:
            cobb::lua::argerror(L, arg_index, "path string or options table expected");
      }
      {  // Remove leading slash(es)
         int i    = 0;
         int size = out.path.size();
         for (; i < size; ++i)
            if (out.path[i] != '/' && out.path[i] != '\\')
               break;
         if (i)
            out.path = out.path.mid(i);
      }
      out.path = QDir::cleanPath(out.path); // resolve "." and ".." as much as possible (e.g. "a/b/../c" -> "a/c") and convert directory separators to '/'
      return out;
   }

   extern int load_and_push_resource(lua_State* L, load_resource_params params) {
      QByteArray buffer;
      if (params.load_from == load_resource_params::source::game_assets) {
         auto path = std::filesystem::path((char8_t*)params.path.toUtf8().data());
         //
         std::unique_ptr<dovah::bsa_archived_file> file = nullptr;
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [path, &file]() {
               file.reset(DovahKitCore::get().lookup_game_asset(path, true));
            };
            send_script_task(*task);
            delete task;
         }
         if (!file)
            return 0;
         buffer = QByteArray((const char*)file->data(), file->size());
      } else if (params.load_from == load_resource_params::source::script_package) {
         if (params.path.startsWith("../"))
            cobb::lua::error(L, "the specified path lies outside the script package folder");
         //
         auto folder = core::subsystems::coordinator::get().package_folder_path();
         if (folder.isEmpty())
            cobb::lua::error(L, "this script is not running as part of a script package");
         QFile file(QDir(folder).absoluteFilePath(params.path));
         if (!file.open(QIODevice::ReadOnly))
            return 0;
         buffer = file.readAll();
      }
      //
      QString filename;
      QString extension;
      {
         int i = params.path.lastIndexOf('/');
         int j = params.path.lastIndexOf('.');
         if (j > i)
            extension = params.path.mid(j + 1).toLower();
         filename = params.path.mid(i + 1);
      }
      QString type = extension;
      if (!params.type.isEmpty())
         type = params.type.toLower();
      //
      auto category = _category::binary;
      {
         for (auto& pair : _formats) {
            if (type == pair.second) {
               category = pair.first;
               break;
            }
         }
      }
      switch (category) {
         case _category::audio:
            //
            // If we ever add support for audio formats, we'd put handling for them here.
            //
            return 0;
         case _category::binary:
            //
            // We default to binary, below.
            //
            break;
         case _category::image:
            //
            // If we ever add support for non-raster image formats, we'd put handling for them here.
            //
            [[fallthrough]];
         case _category::raster:
            if (type == "dds") {
               DovahscriptResourceHandle resource;
               {
                  auto* task    = new tasks::s2m::lambda(true);
                  task->handler = [&buffer, &resource]() {
                     resource = core::subsystems::resources::get().create_resource(buffer, resource_type::dds);
                  };
                  send_script_task(*task);
                  delete task;
               }
               return push_native_object(resource);
            }
            //
            // Try QImageReader:
            //
            {
               QString format  = type;
               bool    matched = true;
               if (type == "bmp") {
                  if (!simple_format_tests::bmp(buffer))
                     return 0;
               } else if (type == "dds") {
               } else if (type == "gif") {
               } else if (type == "png") {
                  if (!simple_format_tests::png(buffer))
                     return 0;
               } else if (type == "jpg" || type == "jpeg") {
                  if (!simple_format_tests::jpg(buffer))
                     return 0;
               } else {
                  matched = false;
               }
               if (!matched) {
                  if (!params.type.isEmpty())
                     return 0;
                  format.clear();
               }
               QBuffer      device = QBuffer(&buffer);
               QImageReader reader;
               reader.setFileName(filename);
               reader.setDevice(&device);
               if (matched)
                  reader.setFormat(format.toLatin1());
               QImage raster = reader.read();
               if (!raster.isNull()) {
                  DovahscriptResourceHandle resource;
                  {
                     auto* task    = new tasks::s2m::lambda(true);
                     task->handler = [&raster, &resource]() {
                        resource = core::subsystems::resources::get().create_resource(raster);
                     };
                     send_script_task(*task);
                     delete task;
                  }
                  return push_native_object(resource);
               }
            }
            //
            // Unknown; break, and read as binary.
            //
            break;
         case _category::text:
            if (type == "utf-8") {
               lua_pushstring(L, QString::fromUtf8(buffer).toUtf8());
               return 1;
            }
            if (type == "utf-16") {
               lua_pushstring(L, QString::fromUtf16((const char16_t*)buffer.data(), buffer.size()).toUtf8());
               return 1;
            }
            lua_pushstring(L, QString(buffer).toUtf8());
            return 1;
      }
      //
      // Default to binary:
      //
      DovahscriptResourceHandle resource;
      {
         auto* task    = new tasks::s2m::lambda(true);
         task->handler = [&buffer, &resource]() {
            resource = core::subsystems::resources::get().create_resource(buffer, resource_type::binary);
         };
         send_script_task(*task);
         delete task;
      }
      return push_native_object(resource);
   }
}