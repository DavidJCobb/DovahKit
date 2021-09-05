#include "ini.h"
#include <QChildEvent>

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
      category(c), name(n), serializationType(st), values{ .initial = value, .current = value }
   {
   }
   Setting::Setting(File& f, const QString& c, const QString& n, const QVariant& value) :
      QObject(&f),
      category(c), name(n), serializationType(_best_serialization_type_for(value)), values{ .initial = value, .current = value }
   {
   }

   File* Setting::file() const noexcept {
      return qobject_cast<File*>(this->parent());
   }

   void Setting::setCurrentValue(const QVariant& v) noexcept {
      auto old = this->values.current;
      this->values.current = v.isValid() ? v : this->values.initial;
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
   #pragma endregion

   #pragma region File
   void File::childEvent(QChildEvent* event) {
      auto* setting = qobject_cast<Setting*>(event->child());
      if (!setting)
         return;
      //
      auto& map = this->_by_category;
      if (event->added()) {
         map[setting->category].push_back(setting);
         return;
      }
      if (event->removed()) {
         auto it = map.find(setting->category);
         if (it == map.end())
            return;
         auto& list = *it;
         auto  i    = list.indexOf(setting);
         if (i >= 0) {
            list.removeAt(i);
            if (list.isEmpty())
               map.erase(it);
         }
         return;
      }
   }
   Setting* File::setting(const QString& category, const QString& name) const noexcept {
      auto& map = this->_by_category;
      auto  it  = map.find(category);
      if (it == map.end())
         return nullptr;
      auto& list = *it;
      for (auto* s : list)
         if (s->name == name)
            return s;
      return nullptr;
   }
   Setting* File::setting(const QString& name) const noexcept {
      for (auto* o : this->children())
         if (auto* s = qobject_cast<Setting*>(o))
            if (s->name == name)
               return s;
      return nullptr;
   }
   QList<QString> File::categoryNames() const noexcept {
      return this->_by_category.keys();
   }
   QVector<Setting*> File::settingsByCategory(const QString& category) const noexcept {
      auto& map = this->_by_category;
      auto  it  = map.find(category);
      if (it == map.end())
         return QVector<Setting*>();
      return *it;
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

   void File::load(const QString& text);
   QString File::save();
   QString File::save(const QString& old); // attempts to preserve the whitespace, comments, etc., of (old) while writing the new values in place
   #pragma endregion
}