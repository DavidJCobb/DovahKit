#include "./worldinput_control_scheme_manager.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"
#include "helpers/unreachable.h"
#include "./builtin_control_schemes/ck_standard.h"
#include "./builtin_control_schemes/debug_wasd.h"
#include "./builtin_control_schemes/reach.h"

#include "editor/ini/main.h"
namespace {
   namespace worldinput_ini_settings {
      using namespace dovahkit::ini::main::worldinput;
   }
}

namespace {
   constexpr const char* const scheme_file_extension = ".dkwi";

   // Note: This INCLUDES the null terminator; the null terminator should be written into the file.
   constexpr const char   file_header_text[]      = "WorldinputControlScheme";
   constexpr const size_t file_header_text_length = std::extent<decltype(file_header_text)>::value; // includes the null terminator
}

namespace dovahkit::subsystems::worldinput {
   namespace {
      const char* _hardcoded_scheme_name(const control_scheme& scheme) {
         if (&scheme == &builtin_control_schemes::ck_standard())
            return "Creation Kit";
         if (&scheme == &builtin_control_schemes::debug_wasd())
            return "Debug WASD";
         if (&scheme == &builtin_control_schemes::reach())
            return "Reach";
         return nullptr;
      }
      const char* _hardcoded_scheme_filename(const control_scheme& scheme) {
         if (&scheme == &builtin_control_schemes::ck_standard())
            return "?ck";
         if (&scheme == &builtin_control_schemes::debug_wasd())
            return "?wasd";
         if (&scheme == &builtin_control_schemes::reach())
            return "?reach";
         return nullptr;
      }
   }

   control_scheme_manager::control_scheme_manager() {
      this->reload_all_control_schemes();
   }

   /*static*/ QString control_scheme_manager::path_for_scheme_folder() {
      return dovahkit::subsystems::options::core::get().get_base_options_path() + "worldinput-control-schemes/";
   }
   /*static*/ QString control_scheme_manager::_path_for_scheme(const scheme_filename_type& name) {
      QDir path(path_for_scheme_folder());
      return path.absoluteFilePath(name + scheme_file_extension);
   }

   /*static*/ cobb::ini::setting& control_scheme_manager::_current_scheme_setting(input_device_type dt) {
      switch (dt) {
         case input_device_type::keyboard_mouse:
            return worldinput_ini_settings::sCurrentControlSchemeKeyboard;
         case input_device_type::xinput:
            return worldinput_ini_settings::sCurrentControlSchemeGamepad;
      }
      cobb::unreachable();
   }

   std::optional<control_scheme> control_scheme_manager::_load_scheme(QFile& file) {
      QByteArray buffer = file.readAll();

      if (!buffer.startsWith(file_header_text))
         return {};
      buffer = buffer.mid(file_header_text_length);

      cobb::bitstreams::reader bitreader((const std::uint8_t*)buffer.data(), buffer.size());

      try {
         return control_scheme::read(bitreader);
      } catch (cobb::bitstreams::exceptions::read_exception& ex) {
         #if _DEBUG
            __debugbreak();
         #endif
         return {};
      }
   }
   void control_scheme_manager::_save_scheme(QFile& file, const control_scheme& src) {
      file.write(file_header_text, file_header_text_length);

      cobb::bitstreams::writer bitwriter;
      src.write(bitwriter);

      file.write((const char*)bitwriter.data(), bitwriter.size());
   }

   void control_scheme_manager::_clear_all_schemes() {
      emit this->beforeReset();
      auto clear_list = [](std::vector<saved_control_scheme*>& list) {
         for (auto* item : list)
            delete item;
         list.clear();
      };
      clear_list(this->_schemes_by_device.gamepad);
      clear_list(this->_schemes_by_device.keyboard);
   }

   bool control_scheme_manager::adjust_scheme_name_by_availability(input_device_type dt, scheme_name_type& name) {
      {
         bool taken = false;
         for (const auto* item : this->_get_schemes_by_device(dt)) {
            if (item->name == name) {
               taken = true;
               break;
            }
         }
         if (!taken)
            return false;
      }

      QStringView candidate = name;
      size_t      alternate = 2;
      if (name.endsWith(')')) {
         auto i = name.lastIndexOf('(');
         if (i > 0) {
            bool ok;
            alternate = name.mid(i, name.size() - i - 2).toInt(&ok);
            if (ok)
               candidate = name.left(i);
            else
               alternate = 2;
         }
      }

      bool adjusted = false;
      while (true) {
         bool  found = false;
         auto& list  = this->_get_schemes_by_device(dt);
         for (const auto* item : list) {
            if (item->name == name) {
               found = true;
               break;
            }
         }
         if (!found)
            break;

         name = candidate.toString() + " (" + QString::number(alternate++) + ")";
         adjusted = true;
         //
         if (name.size() > control_scheme::max_name_length) {
            if (candidate.isEmpty()) {
               //
               // We failed to find a unique name. That isn't a serious problem, since we no longer 
               // use schemes' user-facing names as unique identifiers. This whole function only even 
               // exists because auto-renaming schemes to unique names is user-friendly, but if we 
               // fail to do so, that doesn't break anything (anymore).
               //
               return false;
            }
            candidate = candidate.left(candidate.size() - 1);
            alternate = 2;
            name = candidate.toString() + " (" + QString::number(alternate++) + ")";
         }
      }
      return adjusted;
   }

   // Returns true if the scheme is added. Returns false if that scheme name is taken for that device.
   const control_scheme_manager::saved_control_scheme* control_scheme_manager::add_scheme(const control_scheme& data) {
      auto& list = this->_get_schemes_by_device(data.device_type);
      for(auto* item : list)
         if (item->name == data.name)
            return nullptr;

      constexpr const size_t max_filename_attempts = 20;
      constexpr const size_t filename_length       = 10;

      scheme_filename_type filename;
      size_t filename_attempts = 0;
      for (; filename_attempts < max_filename_attempts; ++filename_attempts) {
         filename.clear();
         filename.resize(filename_length);

         std::srand(std::time(nullptr));
         for (size_t i = 0; i < filename_length; ++i) {
            auto d = std::rand();
            filename[(uint)i] = QChar::fromLatin1((d % 10) + '0');
         }

         QFile file(this->_path_for_scheme(filename));
         if (!file.open(QIODevice::NewOnly)) {
            switch (file.error()) {
               case QFileDevice::PermissionsError:
                  return nullptr; // TODO: throw an exception?
            }
            continue;
         }

         this->_save_scheme(file, data);
         break;
      }
      if (filename_attempts >= max_filename_attempts) {
         return nullptr; // TODO: throw an exception?
      }

      list.push_back(new saved_control_scheme(data));
      list.back()->filename = filename;
      return list.back();
   }

   void control_scheme_manager::overwrite_scheme(const saved_control_scheme* const_dst, const control_scheme& src) {
      assert(const_dst != nullptr);
      assert(!const_dst->is_hardcoded());
      saved_control_scheme* dst = nullptr;
      {
         auto& list = this->_get_schemes_by_device(src.device_type);
         for (auto* item : list) {
            if (item == const_dst) {
               dst = item;
               break;
            }
         }
      }
      assert(dst != nullptr);

      if (*dst == src)
         return;

      auto prior_name = const_dst->name;

      auto filename = dst->filename;
      assert(!filename.isEmpty());
      *dst = src;
      assert(!filename.isEmpty());

      QFile file(this->_path_for_scheme(filename));
      if (!file.open(QIODevice::WriteOnly))
         return; // TODO: throw?

      this->_save_scheme(file, src);

      emit this->controlSchemeModified(prior_name, *dst, this->is_current_scheme(const_dst));
   }

   void control_scheme_manager::delete_scheme(const saved_control_scheme* const_target) {
      assert(const_target != nullptr);
      assert(!const_target->filename.isEmpty());
      assert(const_target->filename[0] != '?'); // hardcoded scheme sentinel

      auto& list = this->_get_schemes_by_device(const_target->device_type);
      auto  it   = std::find(list.begin(), list.end(), const_target);
      assert(it != list.end());

      auto* target = *it;
      list.erase(it);

      auto filename = target->filename;
      delete target;
      QFile file(this->_path_for_scheme(filename));
      file.remove();
   }

   const control_scheme_manager::saved_control_scheme* control_scheme_manager::lookup_scheme(input_device_type dt, const scheme_name_type& name) const {
      if (name.isEmpty())
         return nullptr;
      auto& list = this->_get_schemes_by_device(dt);
      for (auto* item : list)
         if (item->name == name)
            return item;
      return nullptr;
   }

   const control_scheme_manager::saved_control_scheme* control_scheme_manager::lookup_scheme_by_filename(input_device_type dt, const scheme_filename_type& name) const {
      if (name.isEmpty())
         return nullptr;
      auto& list = this->_get_schemes_by_device(dt);
      for (auto* item : list)
         if (item->filename == name)
            return item;
      return nullptr;
   }

   void control_scheme_manager::reload_all_control_schemes() {
      emit this->beforeReloadAll();
      this->_clear_all_schemes();

      {
         auto add_hardcoded_scheme = [this](const control_scheme& scheme) {
            auto& list = this->_get_schemes_by_device(scheme.device_type);
            list.push_back(new saved_control_scheme(scheme));
            list.back()->name     = _hardcoded_scheme_name(scheme);
            list.back()->filename = _hardcoded_scheme_filename(scheme);
         };
         add_hardcoded_scheme(builtin_control_schemes::ck_standard());
         add_hardcoded_scheme(builtin_control_schemes::debug_wasd());
         add_hardcoded_scheme(builtin_control_schemes::reach());
      }
      
      auto it = QDirIterator(path_for_scheme_folder(), QDirIterator::NoIteratorFlags);
      while (it.hasNext()) {
         QString filepath = it.next();
         if (!filepath.endsWith(scheme_file_extension))
            continue;
         
         QFile file(filepath);
         if (!file.open(QIODevice::ReadOnly))
            continue;

         auto loaded = this->_load_scheme(file);
         if (!loaded.has_value())
            continue;

         auto info = QFileInfo(file);
         auto fn   = info.completeBaseName();
         if (fn.isEmpty() || fn[0] == '?') // hardcoded scheme sentinel (should be impossible here)
            continue;

         auto* saved = new saved_control_scheme(std::move(loaded.value()));
         saved->filename = fn;

         this->_get_schemes_by_device(saved->device_type).push_back(saved);
      }

      emit this->reloadedAll();
   }

   const control_scheme& control_scheme_manager::get_current_scheme(input_device_type dt) {
      auto&   setting = _current_scheme_setting(dt);
      QString name;
      {
         auto name_std = setting.get_current_value<std::string>();
         name = QString::fromUtf8(name_std.c_str(), name_std.size());
      }

      const saved_control_scheme* scheme = nullptr;
      if (!name.isEmpty())
         scheme = this->lookup_scheme_by_filename(dt, name);
      if (scheme)
         return *scheme;

      {
         std::string name_std;
         switch (dt) {
            case input_device_type::keyboard_mouse:
            default:
               name_std = _hardcoded_scheme_filename(builtin_control_schemes::debug_wasd());
               break;
            case input_device_type::xinput:
               name_std = _hardcoded_scheme_filename(builtin_control_schemes::reach());
               break;
         }
         name = QString::fromUtf8(name_std.c_str(), name_std.size());
      }
      scheme = this->lookup_scheme_by_filename(dt, name);
      assert(scheme != nullptr);
      //
      this->set_current_scheme(scheme);
      //
      return *scheme;
   }
   void control_scheme_manager::set_current_scheme(const saved_control_scheme* scheme) {
      if (!scheme)
         return;
      auto dt = scheme->device_type;

      #if _DEBUG
      {
         auto& list  = this->_get_schemes_by_device(dt);
         bool  found = false;
         for (auto* item : list) {
            if (item == scheme) {
               found = true;
               break;
            }
         }
         assert(found);
      }
      #endif

      auto& setting = _current_scheme_setting(dt);

      auto current = setting.get_current_value<std::string>();
      auto desired = scheme->filename.toUtf8().toStdString();
      if (current == desired)
         return;

      setting.set_current_value<std::string>(desired);
      emit currentSchemeChanged(*scheme);
   }

   bool control_scheme_manager::is_current_scheme(const saved_control_scheme* scheme) {
      if (!scheme)
         return false;

      auto& current = this->get_current_scheme(scheme->device_type);
      return (&current == scheme);
   }
}