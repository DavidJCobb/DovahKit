#include "dovah.h"
#include "../../../../helpers/lua/setfuncs.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/lua_managed_resources.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../util.h"
#include "../../wrapper_util.h"
#include "../../classes/benchmark.h"

#include "../../../core.h" // DovahKitCore
#include "../form_type_values.h"
#include "../../cross_thread_tasks/s2m/create_form.h"
#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../cross_thread_tasks/s2m/log_message.h"
#include "../../cross_thread_tasks/s2m/test_call_and_response.h"
#include "../../wrappers/form.h"

#include <QBuffer>
#include <QImageReader>
#include "../../../../../DirectXTex/DirectXTex.h"
#include "../../../../dovah/files/bsa/bsa_archived_file.h"
#include "../../wrappers/resource/raster.h"
#include "../../wrappers/resource/unknown.h"

namespace {
   using namespace editor_script;

   namespace _definitions {
      luastackchange_t benchmark_start(lua_State* L) {
         auto* p = lua_newuserdata(L, sizeof(classes::benchmark)); // push 1
         luaL_getmetatable(L, classes::benchmark::metatable_key); // push 1
         lua_setmetatable(L, -2); // pop 1
         new (p) classes::benchmark;
         return 1;
      }
      luastackchange_t benchmark_stop(lua_State* L) {
         auto* self = (classes::benchmark*) editor_script::cast_to_exact_class(L, 1, classes::benchmark::metatable_key);
         if (self == nullptr) {
            luaL_error(L, "bad argument #1 to dovah.benchmark_stop (expected %s)", classes::benchmark::metatable_key);
         }
         __assume(self != nullptr);
         self->finish();
         return 0;
      }
      luastackchange_t count_forms_of_type(lua_State* L) {
         auto& editor = DovahKitCore::get();
         if (!editor.has_data()) {
            lua_pushinteger(L, 0);
            return 1;
         }
         //
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
         if (!valid)
            return 0;
         auto& info = dovah::form_type_info::lookup(ft);
         if (info.flags & dovah::form_type_info::flag::is_singleton) {
            lua_pushinteger(L, 1);
            return 1;
         }
         lua_pushinteger(L, editor.count_forms_of_type(ft));
         return 1;
      }
      luastackchange_t create_form(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            luaL_error(L, "cannot create a new form because no data is loaded in the editor");
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
         if (!valid)
            luaL_error(L, "cannot create a new form because no valid form type was supplied");
            
         auto* m = new tasks::s2m::create_form;
         m->form_type = ft;
         if (lua_gettop(L) > 1 && lua_type(L, 2) == LUA_TTABLE) { // if an options table was passed
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "parent");
            if (!lua_isnoneornil(L, 3)) {
               auto* wrap = (wrapper*)editor_script::cast_to_class(L, 3, wrappers::form::metatable_key);
               if (wrap) {
                  m->parent = wrap->stub;
               } else {
                  lua_warning(L, "dovah.create_form() call tried to specify a parent but didn't pass a form", 0);
               }
            }
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "grid_coordinates");
            if (!lua_isnoneornil(L, 3)) {
               if (lua_type(L, 3) == LUA_TTABLE) {
                  lua_getfield(L, 3, "x");
                  lua_getfield(L, 3, "y");
                  m->cell_grid_coordinates.x = lua_tonumber(L, 4);
                  m->cell_grid_coordinates.y = lua_tonumber(L, 5);
               } else {
                  lua_warning(L, "dovah.create_form() call tried to specify grid coordinates for an exterior cell, but didn't pass valid numbers", 0);
               }
            }
            lua_settop(L, 2);
            //
            lua_getfield(L, 2, "editor_id");
            if (!lua_isnoneornil(L, 3)) {
               m->editorID = luaL_tolstring(L, 3, nullptr);
            }
            lua_settop(L, 2);
         }
         DovahKitScriptVMMessenger::get().send_message(m);
         if (m->error) {
            if (!m->error_text)
               m->error_text = "";
            luaL_error(L, m->error_text);
         }
         auto* stub = m->result;
         delete m;
         //
         wrapper out;
         auto* mt = wrap_form(out, stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t for_each_form_of_type(lua_State* L) {
         luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         //
         bool  valid = false;
         auto  ft    = editor_script::get_form_type_from_stack(L, 1, valid);
         if (!valid)
            return 0;
         auto& info  = dovah::form_type_info::lookup(ft);
         if (info.flags & dovah::form_type_info::flag::is_singleton) {
            //
            // For singleton forms, only use the canonical stub.
            //
            auto* stub = editor.get_singleton_form(ft, false);
            if (stub) {
               lua_pushvalue(L, 2); // push the function
               wrapper out;
               auto*   mt = wrap_form(out, stub);
               if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt))
                  lua_call(L, 1, 1);
            }
            return 0;
         }
         //
         editor.for_each_form_of_type(ft, [L](dovah::form_stub* stub) {
            lua_pushvalue(L, 2); // push the function
            wrapper out;
            auto*   mt = wrap_form(out, stub);
            if (DovahKitScriptVMUserdataInterface::get().push(L, out, mt)) {
               lua_call(L, 1, 1);
               if (lua_toboolean(L, -1) == 1) {
                  return true;
               }
            } else {
               lua_settop(L, 2);
            }
            return false;
         });
         //
         return 0;
      }
      luastackchange_t get_form_by_id(lua_State* L) {
         luaL_argcheck(L, lua_isnumber(L, 1), 1, "form ID (number) expected");
         auto& editor = DovahKitCore::get();
         if (!editor.has_data())
            return 0;
         int  isnum;
         auto id = lua_tointegerx(L, 1, &isnum);
         if (!isnum)
            return 0;
         if (id < 0 || id > 0xFFFFFFFF)
            return 0;
         auto* stub = editor.get_form(id);
         if (!stub)
            return 0;
         //
         wrapper out;
         auto*   mt = wrap_form(out, stub);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t log_message(lua_State* L) {
         auto m = new editor_script::tasks::s2m::log_message();
         //
         auto argcount = lua_gettop(L);
         if (!argcount)
            return 0;
         //
         if (lua_type(L, 1) != LUA_TSTRING) { // coerce argument 1 to a string if it isn't one, as string.format doesn't do this automatically
            luaL_tolstring(L, 1, nullptr);
            lua_copy(L, argcount + 1, 1);
            lua_pop(L, 1);
         }
         //
         lua_getfield(L, LUA_REGISTRYINDEX, DovahKitScriptVMCore::string_format_registry_key);
         if (lua_isfunction(L, argcount + 1)) {
            lua_rotate(L, 1, 1); // move (string.format) ahead of the other stack elements
            lua_call  (L, argcount, 1);
         }
         //
         const char* out = lua_tostring(L, 1);
         if (!out) {
            out = "";
         }
         m->text = QString::fromUtf8(out);
         //
         DovahKitScriptVMMessenger::get().send_message(m);
         return 0;
      }
      luastackchange_t lookup_game_asset(lua_State* L) {
         const char* raw = nullptr;
         if (lua_isstring(L, 1)) {
            raw = lua_tostring(L, 1);
         } else if (lua_istable(L, 1) || lua_isuserdata(L, 1)) {
            int type = luaL_getmetafield(L, 1, "__tostring");
            lua_pop(L, 1);
            if (type == LUA_TFUNCTION)
               raw = luaL_tolstring(L, 1, nullptr);
         }
         luaL_argcheck(L, raw != nullptr, 1, "string expected");
         std::filesystem::path path = raw;
         //
         std::unique_ptr<dovah::bsa_archived_file> file = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [path, &file]() {
               file.reset(DovahKitCore::get().lookup_game_asset(path, true));
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!file)
            return 0;
         {
            QImageReader reader;
            {
               auto p = path.filename().u8string();
               auto s = QString::fromUtf8((const char*)p.c_str());
               reader.setFileName(s);
            }
            auto buffer = QByteArray::fromRawData((const char*)file->data(), file->size());
            auto device = QBuffer(&buffer);
            reader.setDevice(&device);
            if (!reader.format().isEmpty()) {
               auto raster = reader.read();
               if (!raster.isNull()) {
                  LuaManagedResource* resource = nullptr;
                  {
                     auto* task    = new tasks::s2m::lambda(true);
                     task->handler = [&raster, &resource]() {
                        resource = DovahKitScriptVMResourceInterface::get().create_resource(raster);
                     };
                     DovahKitScriptVMUITaskConduit::get().send_message(*task);
                     delete task;
                     assert(resource);
                  }
                  return wrappers::resource::raster::wrap_and_push(L, *resource);
               }
            }
         }
         if (_stricmp(path.extension().string().data(), ".dds") == 0) {
            LuaManagedResource* resource = nullptr;
            {
               auto* task    = new tasks::s2m::lambda(true);
               task->handler = [&file, &resource]() {
                  using namespace DirectX;
                  using image_ptr_t = std::unique_ptr<ScratchImage>;
                  static constexpr DXGI_FORMAT DESIRED_DX_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
                  //
                  TexMetadata metadata;
                  image_ptr_t raw(new (std::nothrow) ScratchImage);
                  HRESULT     hr = LoadFromDDSMemory(file->data(), file->size(), DDS_FLAGS_NONE, &metadata, *raw);
                  if (FAILED(hr))
                     return;
                  //
                  if (IsTypeless(metadata.format)) {
                     metadata.format = MakeTypelessUNORM(metadata.format);
                     if (IsTypeless(metadata.format))
                        return;
                     raw->OverrideFormat(metadata.format);
                  }
                  if (IsPlanar(metadata.format)) {
                     //
                     // Some DDS files split the image into multiple "planes:" instead of having the R, G, B, and A 
                     // values interleaved together, the file effectively stores four single-channel images. We want 
                     // to merge those into RGBA.
                     //
                     image_ptr_t merged(new (std::nothrow) ScratchImage);
                     if (!merged)
                        return; // out of memory
                     hr = ConvertToSinglePlane(raw->GetImages(), raw->GetImageCount(), metadata, *merged);
                     if (FAILED(hr))
                        return;
                     metadata = merged->GetMetadata();
                     raw.swap(merged);
                  }
                  //
                  if (IsCompressed(metadata.format)) {
                     image_ptr_t decompressed(new (std::nothrow) ScratchImage);
                     if (!decompressed)
                        return; // out of memory
                     Decompress(raw->GetImages(), raw->GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, *decompressed);
                     std::swap(decompressed, raw);
                     metadata = raw->GetMetadata();
                  }
                  if (metadata.format != DESIRED_DX_FORMAT) {
                     image_ptr_t converted(new (std::nothrow) ScratchImage);
                     if (!converted)
                        return; // out of memory
                     hr = Convert(raw->GetImages(), raw->GetImageCount(), metadata, DESIRED_DX_FORMAT, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *converted);
                     if (FAILED(hr))
                        return;
                     std::swap(converted, raw);
                     metadata = raw->GetMetadata();
                  }
                  //
                  if (HasAlpha(metadata.format) && metadata.IsPMAlpha()) {
                     //
                     // If alpha needs to be premultiplied, handle it. Note that PremultiplyAlpha returns an 
                     // error code on images that don't need PMA, so we actually do have to check that.
                     //
                     image_ptr_t mod(new (std::nothrow) ScratchImage);
                     if (!mod)
                        return; // out-of-memory
                     hr = PremultiplyAlpha(raw->GetImages(), raw->GetImageCount(), metadata, TEX_PMALPHA_REVERSE, *mod);
                     if (FAILED(hr))
                        return;
                     metadata = mod->GetMetadata();
                     raw.swap(mod);
                  }
                  //
                  if (metadata.IsCubemap()) {
                     //
                     // Don't care; but a proper DDS wrapper might provide individual access to each cubemap 
                     // face.
                     //
                  }
                  //
                  const auto* first_layer = raw->GetImage(0, 0, 0);
                  if (!first_layer)
                     return;
                  if (first_layer->width > std::numeric_limits<int>::max())
                     return;
                  if (first_layer->height > std::numeric_limits<int>::max())
                     return;
                  if (first_layer->rowPitch > std::numeric_limits<int>::max())
                     return;
                  auto data = QImage((const uchar*)first_layer->pixels, first_layer->width, first_layer->height, first_layer->rowPitch, QImage::Format_ARGB32);
                  assert(!data.isNull());
                  data.detach();
                  assert(data.isDetached());
                  assert(data.constBits() != first_layer->pixels);
                  resource = DovahKitScriptVMResourceInterface::get().create_resource(data);
               };
               DovahKitScriptVMUITaskConduit::get().send_message(*task);
               delete task;
            }
            if (resource) {
               return wrappers::resource::raster::wrap_and_push(L, *resource);
            }
         }
         //
         // The resource could not be identified.
         //
         LuaManagedResource* resource = nullptr;
         {
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [&file, &resource]() {
               resource = DovahKitScriptVMResourceInterface::get().create_resource(QByteArray::fromRawData((const char*)file->data(), file->size()));
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            assert(resource);
         }
         wrapper out;
         out.type = wrapper_type::lua_managed_resource;
         out.managed_resource = resource;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::resource::unknown::metatable_key);
      }
      luastackchange_t object_is(lua_State* L) {
         lua_settop(L, 2);
         constexpr int index_obj = 1;
         constexpr int index_req = 2;
         constexpr int index_mt  = 3;
         //
         luaL_argcheck(L, lua_isstring(L, index_req), 2, "typename (string) expected");
         auto  t   = lua_type    (L, index_obj);
         auto* req = lua_tostring(L, index_req);
         if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
            lua_getmetatable(L, index_obj);
            assert(lua_gettop(L) == index_mt);
            while (lua_type(L, index_mt) == LUA_TTABLE) {
               lua_pushstring(L, "__name");
               if (lua_rawget(L, index_mt) == LUA_TSTRING) {
                  auto* tn = lua_tostring(L, -1);
                  if (strcmp(tn, req) == 0) {
                     lua_pushboolean(L, true);
                     return 1;
                  }
               }
               lua_settop(L, index_mt);
               lua_pushstring(L, "__superclass");
               lua_rawget    (L, index_mt);
               lua_replace(L, index_mt);
            }
         }
         const char* tn  = lua_typename(L, t);
         lua_pushboolean(L, strcmp(tn, req) == 0);
         return 1;
      }
      luastackchange_t test_call_and_response(lua_State* L) {
         auto* m = new editor_script::tasks::s2m::test_call_and_response();
         DovahKitScriptVMMessenger::get().send_message(m);
         return 0;
      }
      luastackchange_t type(lua_State* L) {
         lua_settop(L, 1);
         lua_checkstack(L, 3);
         auto t = lua_type(L, 1);
         if (t == LUA_TTABLE || t == LUA_TUSERDATA) {
            if (lua_getmetatable(L, 1)) {
               lua_pushstring(L, "__name");
               auto nt = lua_rawget(L, 2);
               if (nt == LUA_TSTRING)
                  return 1;
            }
         }
         lua_pushstring(L, lua_typename(L, t));
         return 1;
      }
   }

   const std::initializer_list<luaL_Reg> _functions = {
      luaL_Reg{ "benchmark_start",        &_definitions::benchmark_start },
      luaL_Reg{ "benchmark_stop",         &_definitions::benchmark_stop },
      luaL_Reg{ "count_forms_of_type",    &_definitions::count_forms_of_type },
      luaL_Reg{ "create_form",            &_definitions::create_form },
      luaL_Reg{ "for_each_form_of_type",  &_definitions::for_each_form_of_type },
      luaL_Reg{ "get_form_by_id",         &_definitions::get_form_by_id },
      luaL_Reg{ "log_message",            &_definitions::log_message },
      luaL_Reg{ "lookup_game_asset",      &_definitions::lookup_game_asset },
      luaL_Reg{ "object_is",              &_definitions::object_is },
      luaL_Reg{ "test_call_and_response", &_definitions::test_call_and_response },
      luaL_Reg{ "type",                   &_definitions::type },
   };
}
namespace editor_script::namespace_setup {
   extern void dovah(lua_State* L) {
      cobb::lua::setfuncs(L, _functions);
   }
}
