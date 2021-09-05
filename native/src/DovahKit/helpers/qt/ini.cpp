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
      category(c), name(n), serializationType(st != SettingSerializationType::Undefined ? st : _best_serialization_type_for(value)), values{ .initial = value, .current = value }
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
   File::File(std::initializer_list<_CategoryConstructParams> cats, QObject* parent) : QObject(parent) {
      for (auto& cat : cats) {
         QString cn = cat.name;
         this->_by_category[cn].reserve(cat.settings.size());
         for (auto& s : cat.settings)
            new Setting(*this, cn, s.name, s.serializationType, s.value);
      }
   }

   void File::childEvent(QChildEvent* event) {
      auto* setting = qobject_cast<Setting*>(event->child());
      if (!setting)
         return;

      
      static_assert(false, "This won't work.");
      //
      // Per Qt documentation:
      // 
      //    Child events are sent immediately to objects when children are added or removed.
      // 
      //    In both cases you can only rely on the child being a QObject(or , if QObject::isWidgetType() 
      //    returns true, a QWidget).This is because in the QEvent::ChildAdded case the child is not yet 
      //    fully constructed; in the QEvent::ChildRemoved case it might have already been destructed.
      //
      // We need another way to maintain the by-category cache.
      // 
      // I think we're gonna have to avoid using the QObject parenting mechanism, in favor of...
      // 
      //  - Settings need to call a member function on a File to register themselves when created.
      // 
      //  - Files need to hook the Setting::destroyed signal and unregister the Setting when it is 
      //    destroyed.
      // 
      //  - When a File is destroyed, it should unregister all Settings and sever any Setting::destroyed 
      //    signals it requested.
      // 
      //  - File::setting(const QString& name) will need to be modified to not crawl the QObject child 
      //    list. Ditto for File::commitPendingChanges() and File::discardPendingChanges().
      // 
      //  - This event handler will need to be removed.
      //


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