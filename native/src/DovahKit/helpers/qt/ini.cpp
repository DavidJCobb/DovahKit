#include "ini.h"
#include <QChildEvent>

namespace {
   QString _extractCategoryName(QStringRef view, QChar comment_char) {
      assert(view[0] == '[');
      int depth = 1;
      int last  = -1;
      for (int j = 1; j < view.size(); ++j) {
         if (view[j] == '[') {
            ++depth;
         } else if (view[j] == ']') {
            last = j;
            --depth;
         } else if (view[j] == comment_char)
            break;
      }
      if (depth == 0) {
         assert(last > 0);
         return view.mid(1, last - 1).toString();
      }
      return QString();
   }
}

namespace cobb::qt::ini {
   namespace {
      static SettingSerializationType _best_serialization_type_for(const QVariant& v) {
         switch (v.userType()) {
            case QMetaType::Bool:
               return SettingSerializationType::Bool;
            case QMetaType::Double:
               return SettingSerializationType::Double;
            case QMetaType::Float:
               return SettingSerializationType::Float;
            case QMetaType::Int:
               return SettingSerializationType::Integer;
            case QMetaType::UInt:
               return SettingSerializationType::IntegerUnsigned;
            case QMetaType::QString:
               return SettingSerializationType::String;
         }
         return SettingSerializationType::String;
      }
   }

   #pragma region Setting
   Setting::Setting(File& f, const QString& c, const QString& n, SettingSerializationType st, const QVariant& value) :
      QObject(&f),
      category(c),
      name(n),
      serializationType(st != SettingSerializationType::Undefined ? st : _best_serialization_type_for(value)),
      qMetaTypeID(value.userType()),
      values{ .initial = value, .current = value }
   {
      f._addSetting(cobb::passkey<File, Setting>(), *this);
   }
   Setting::Setting(File& f, const QString& c, const QString& n, const QVariant& value) :
      QObject(&f),
      category(c),
      name(n),
      serializationType(_best_serialization_type_for(value)),
      qMetaTypeID(value.userType()),
      values{ .initial = value, .current = value }
   {
      f._addSetting(cobb::passkey<File, Setting>(), *this);
   }

   File* Setting::file() const noexcept {
      return qobject_cast<File*>(this->parent());
   }

   void Setting::setCurrentValue(const QVariant& v) noexcept {
      auto old = this->values.current;
      this->values.current = v.isValid() ? v : this->values.initial;
      if (this->values.current != old)
         emit valueChanged(old, this->values.current);
   }
   void Setting::setPendingValue(const QVariant& v) noexcept {
      this->values.pending = v;
   }

   void Setting::discardPendingValue() {
      if (!this->values.pending.isValid())
         return;
      this->values.pending = QVariant();
      emit pendingValueDiscarded();
   }
   void Setting::commitPendingValue() {
      if (!this->values.pending.isValid())
         return;
      QVariant old = this->values.current;
      this->values.current = std::move(this->values.pending);
      this->values.pending = QVariant();
      emit pendingValueCommitted();
      emit valueChanged(old, this->values.current);
   }

   void Setting::load(QStringRef view) {
      view = view.trimmed();
      //
      bool ok;
      switch (this->serializationType) {
         case SettingSerializationType::Bool:
            if (view.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0) {
               this->setCurrentValue(false);
               return;
            }
            if (view.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0) {
               this->setCurrentValue(true);
               return;
            }
            {
               auto v = view.toDouble(&ok);
               if (ok) {
                  this->setCurrentValue(v != 0.0);
                  return;
               }
            }
            break;
         case SettingSerializationType::Double:
            {
               auto v = view.toDouble(&ok);
               if (ok) {
                  this->setCurrentValue(v);
                  return;
               }
            }
            break;
         case SettingSerializationType::Float:
            {
               auto v = view.toFloat(&ok);
               if (ok) {
                  this->setCurrentValue(v);
                  return;
               }
            }
            break;
         case SettingSerializationType::Integer:
            {
               auto v = view.toInt(&ok);
               if (ok) {
                  this->setCurrentValue(v);
                  return;
               }
            }
            break;
         case SettingSerializationType::IntegerUnsigned:
            {
               auto v = view.toUInt(&ok);
               if (ok) {
                  this->setCurrentValue(v);
                  return;
               }
            }
            break;
         case SettingSerializationType::String:
            if (this->qMetaTypeID == QMetaType::QString) {
               this->setCurrentValue(view.toString());
               return;
            }
            {  // Handle arbitrary structs.
               QVariant v = view.toString();
               if (v.convert(this->qMetaTypeID)) {
                  this->setCurrentValue(v);
                  return;
               }
            }
            break;
      }
      //
      // For bad values, reset to the default:
      //
      this->setCurrentValue(QVariant());
   }
   QString Setting::currentValueString() const noexcept {
      switch (this->serializationType) {
         case SettingSerializationType::Bool:
            return this->values.current.toBool() ? "true" : "false";
         case SettingSerializationType::Double:
            return QString::number(this->values.current.toDouble());
         case SettingSerializationType::Float:
            return QString::number(this->values.current.toFloat());
         case SettingSerializationType::Integer:
            return QString::number(this->values.current.toInt());
         case SettingSerializationType::IntegerUnsigned:
            return QString::number(this->values.current.toUInt());
         case SettingSerializationType::String:
            return this->values.current.toString();
      }
      return QString();
   }
   #pragma endregion

   #pragma region File
   File::File(std::initializer_list<_CategoryConstructParams> cats, QObject* parent) : QObject(parent) {
      for (auto& cat : cats) {
         QString cn = cat.name;
         this->_by_category[cn].reserve(cat.settings.size());
         for (auto& s : cat.settings)
            new Setting(*this, cn, s.name, s.serializationType, s.value);
      }
   }

   void File::_addSetting(cobb::passkey<File, Setting>, Setting& setting) {
      this->_all_settings.push_back(&setting);
      this->_by_category[setting.category].push_back(&setting);
   }

   Setting* File::setting(const QString& category, const QString& name, Qt::CaseSensitivity ci) const noexcept {
      if (ci == Qt::CaseSensitive)
         return this->setting(category, name);
      //
      auto& map = this->_by_category;
      for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
         const auto& key  = it->first;
         const auto& list = it->second;
         if (key.compare(category, Qt::CaseInsensitive) == 0)
            for (Setting* s : list)
               if (s->name.compare(name, Qt::CaseInsensitive) == 0)
                  return s;
      }
      return nullptr;
   }
   Setting* File::setting(const QString& category, const QString& name) const noexcept {
      auto& map = this->_by_category;
      auto  it  = map.find(category);
      if (it == map.end())
         return nullptr;
      auto& list = *it;
      for (Setting* s : list)
         if (s->name == name)
            return s;
      return nullptr;
   }
   Setting* File::setting(const QString& name) const noexcept {
      for (Setting* s : this->_all_settings)
         if (s->name == name)
            return s;
      return nullptr;
   }
   QList<QString> File::categoryNames() const noexcept {
      return this->_by_category.keys();
   }
   QVector<Setting*> File::settingsByCategory(const QString& category) const noexcept {
      QVector<Setting*> out;
      //
      auto& map = this->_by_category;
      auto  it  = map.find(category);
      if (it == map.end())
         return out;
      auto& list = *it;
      auto  size = list.size();
      out.resize(size);
      for (decltype(size) i = 0; i < size; ++i)
         out[i] = list[i];
      return out;
   }

   QString File::categoryNameCanonicalCase(const QString& name) const noexcept {
      auto& map = this->_by_category;
      for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
         const auto& key  = it->first;
         const auto& list = it->second;
         if (key.compare(name, Qt::CaseInsensitive) == 0)
            return key;
      }
      return QString();
   }

   void File::commitPendingChanges() {
      for (auto* o : this->children())
         if (auto* s = qobject_cast<Setting*>(o))
            s->commitPendingValue();
   }
   void File::discardPendingChanges() {
      for (auto* o : this->children())
         if (auto* s = qobject_cast<Setting*>(o))
            s->discardPendingValue();
   }

   void File::load(const QString& text) {
      QString category;
      //
      int i    = 0;
      int size = text.size();
      while (i < size) {
         QStringRef line;
         {
            int j = text.indexOf('\n', i);
            if (j < 0)
               j = size;
            line = QStringRef(&text, i, j - i).trimmed();
            i = j;
         }
         if (line.isEmpty())
            continue;
         //
         if (line[0] == '[') { // category?
            QString cn = _extractCategoryName(line, this->_comment_char);
            if (cn.isEmpty()) // bad line (unbalanced braces, etc.)
               continue;
            category = cn;
            continue;
         }
         //
         QString key;
         bool    equal = false;
         int     j     = 0;
         for (; j < line.size(); ++j) {
            QChar c = line[j];
            if (c == this->_comment_char)
               break;
            if (c == '=') {
               equal = true;
               break;
            }
            key += c;
         }
         if (!equal || key.isEmpty())
            continue;
         auto* setting = this->setting(category, key, Qt::CaseInsensitive);
         if (!setting)
            continue;
         //
         ++j; // skip the equal sign
         //
         int k;
         if (setting->serializationType != SettingSerializationType::String) {
            k = line.indexOf(this->_comment_char, j);
            if (k < 0)
               k = line.size();
         } else {
            k = line.size();
         }
         setting->load(line.mid(j, k - j));
      }
   }

   QString File::save() {
      QString out;
      //
      auto& map = this->_by_category;
      //
      {  // Serialize settings not in any category first
         auto it = map.find("");
         if (it != map.end()) {
            const auto& list = *it;
            for (Setting* s : list) {
               if (!out.isEmpty())
                  out += '\n';
               out += QString("%1=%2").arg(s->name).arg(s->currentValueString());
            }
         }
      }
      //
      for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
         const auto& cat  = it->first;
         const auto& list = it->second;
         if (cat.isEmpty())
            continue;
         if (!out.isEmpty())
            out += "\n\n";
         out += QString("[%1]").arg(cat);
         //
         for (Setting* s : list) {
            if (!out.isEmpty())
               out += '\n';
            out += QString("%1=%2").arg(s->name).arg(s->currentValueString());
         }
      }
      //
      return out;
   }

   namespace {
      struct _PendingCategory {
         QString name;
         QString header; // header line, including whitespace and comments
         QString body;
         QVector<const Setting*> found_settings;

         _PendingCategory() {}
         _PendingCategory(const QString& n, const QString& h) : name(n), header(h) {}

         QString toString() const noexcept {
            return QString("%1\n%2").arg(this->header).arg(this->body);
         }
      };
   }
   QString File::save(const QString& old) { // attempts to preserve the whitespace, comments, etc., of (old) while writing the new values in place
      QString out;
      //
      _PendingCategory current;
      QList<QString>   missing = this->categoryNames();
      //
      bool before_categories = true;
      int i    = 0;
      int size = old.size();
      while (i < size) {
         QStringRef line;
         {
            int j = old.indexOf('\n', i);
            if (j < 0)
               j = size;
            line = QStringRef(&old, i, j - i).trimmed();
            i = j;
         }
         if (line.trimmed().isEmpty()) {
            current.body += line;
            continue;
         }
         //
         if (line.startsWith('[')) {
            auto cn = _extractCategoryName(line, this->_comment_char);
            if (cn.isEmpty()) { // bad line
               current.body += line;
               continue;
            }
            //
            // We've opened a new category, so let's write the string content for the previous one, including 
            // any settings that weren't originally in the file before:
            //
            out += current.toString();
            for (Setting* s : this->settingsByCategory(current.name)) {
               if (current.found_settings.contains(s))
                  continue;
               out += s->name;
               out += '=';
               out += s->currentValueString();
               out += '\n';
            }
            //
            // Now let's open the new category:
            //
            current = _PendingCategory(this->categoryNameCanonicalCase(cn), line.toString());
            missing.removeOne(current.name);
            continue;
         }
         //
         int comment_at = line.indexOf(this->_comment_char);
         int equal_at   = line.indexOf('=');
         if (equal_at < 0 || (comment_at >= 0 && equal_at > comment_at)) { // invalid line, entire line is a comment, etc.
            current.body += line;
            continue;
         }
         QString  name    = line.mid(0, equal_at).trimmed().toString();
         Setting* setting = this->setting(current.name, name, Qt::CaseInsensitive);
         if (!setting) {
            current.body += line;
            continue;
         }
         current.found_settings.push_back(setting);
         current.body += name;
         if (name.size() < equal_at) // e.g. "sSetting  = 5"
            current.body += line.mid(name.size(), equal_at - name.size());
         current.body += '=';
         //
         // Preserve any whitespace following the original value (non-string settings only; for strings, assume the 
         // whitespace is part of the value and discard it):
         //
         if (setting->serializationType != SettingSerializationType::String) {
            int pos;
            for (pos = equal_at + 1; pos < line.size(); ++pos)
               if (!line[pos].isSpace())
                  break;
            if (pos > equal_at + 1)
               current.body += line.mid(equal_at + 1, pos - (equal_at + 1));
         }
         //
         // Append value:
         //
         current.body += setting->currentValueString();
         //
         // For non-strings, grab trailing whitespace and comments:
         //
         if (setting->serializationType != SettingSerializationType::String) {
            int first_space_at = -1;
            if (comment_at > 0) {
               if (line[comment_at - 1].isSpace()) {
                  for (first_space_at = comment_at - 1; first_space_at > 0; --first_space_at)
                     if (!line[first_space_at].isSpace())
                        break;
                  current.body += line.mid(first_space_at);
               } else {
                  current.body += line.mid(comment_at);
               }
            } else {
               if (line.back().isSpace()) {
                  for (first_space_at = line.size() - 1; first_space_at > 0; --first_space_at)
                     if (!line[first_space_at].isSpace())
                        break;
                  current.body += line.mid(first_space_at);
               }
            }
         }
         // Done processing this line.
         current.body += '\n';
      }
      out += current.toString();
      for (Setting* s : this->settingsByCategory(current.name)) {
         if (current.found_settings.contains(s))
            continue;
         out += s->name;
         out += '=';
         out += s->currentValueString();
         out += '\n';
      }
      //
      // Write any missing categories:
      //
      for (const auto& cat : missing) {
         if (!out.isEmpty())
            out += "\n\n";
         out += QString("[%1]\n").arg(cat);
         for (Setting* s : this->settingsByCategory(cat)) {
            out += s->name;
            out += '=';
            out += s->currentValueString();
            out += '\n';
         }
      }
      //
      // And now we're done!
      //
      return out;
   }
   #pragma endregion
}