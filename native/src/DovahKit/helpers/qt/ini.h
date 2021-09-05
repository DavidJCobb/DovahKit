#pragma once
#include <QObject>
#include <QVariant>

//
// TODO:
// 
//  - Settings with structs should rely on Qt meta-type converters for serialization and loading; see answers at:
//    <https://stackoverflow.com/questions/23984421/custom-type-in-qvariant-converts-to-empty-string>
// 
//  - File load/save code
// 
// NOTES:
// 
//  - Code which performs some long task in response to a setting being changed (e.g. changing the application 
//    stylesheet) is encouraged to use a Qt::QueuedConnection to listen for the setting change. This way, if the 
//    application commits pending changes and then saves the INI file, it can be sure that the file will be saved 
//    before the long-running task happens.
// 
//  - Qt really doesn't like it when widgets, at the very least, are static local variables, rather than being 
//    local to the main loop (for the main window) or static heap-allocated variables. What impact will that have 
//    on defining settings? Should we block locally allocating them (i.e. by privating the destructor)?
// 
//    What's more, static instances of a QObject outside of a function are vulnerable to the static initialization 
//    order fiasco: the QObject instances can be created before their own QMetaObjects (i.e. Qt RTTI) are created.
// 
//     - Same concern is present for files.
// 
//        - The File() constructor that takes setting definitions may help. We can have a singleton-style getter 
//          function for an INI file which heap-allocates a File with setting definitions; that'll also create 
//          its settings; and then we can...
// 
//           - ...only ever access settings through the File.
// 
//           - ...have specific pieces of code that need a setting use their own getter functions, which get the 
//             setting through the file and then cache the setting pointer in a static local variable for faster 
//             access later.
// 
//     - We also want to support constructing INI file specs at run-time. I want to be able to define all of 
//       Skyrim's INI settings and their default values within the `dovah` folder in a Qt-independent manner, 
//       even if I have the frontend, Dovahscript, render window, etc., all rely on this system here to actually 
//       load and work with Skyrim.ini.
//

namespace cobb::qt::ini {
   enum class SettingSerializationType {
      Undefined = -1,
      Bool,
      Double,
      Float,
      Integer,
      IntegerUnsigned,
      String,
   };

   class File;

   class Setting : public QObject {
      Q_OBJECT;
      public:
         Setting(File&, const QString& category, const QString& name, SettingSerializationType st, const QVariant& value);
         Setting(File&, const QString& category, const QString& name, const QVariant& value); // assumes ST from the variant; struct variants = string

         const QString name;     // case-sensitive at run-time, but different cases are accepted when loading from a file
         const QString category; // case-sensitive at run-time, but different cases are accepted when loading from a file
         const SettingSerializationType serializationType = SettingSerializationType::Undefined;
      protected:
         struct {
            const QVariant initial;
            QVariant current;
            QVariant pending; // invalid variant == no pending changes
         } values;

      public:
         File* file() const noexcept;

         inline QVariant initialValue() const noexcept { return this->values.initial; }
         inline QVariant currentValue() const noexcept { return this->values.current; }
         inline QVariant pendingValue() const noexcept { return this->values.pending; }
         void setCurrentValue(const QVariant&) noexcept; // an invalid variant resets the value to its initial
         void setPendingValue(const QVariant&) noexcept; // an invalid variant clears the pending value

         void discardPendingValue();
         void commitPendingValue();

      signals:
         void pendingValueDiscarded();
         void pendingValueCommitted();
         void valueChanged(QVariant old, const QVariant& now);
   };
   
   // Each Setting is a child-QObject of the File.
   class File : public QObject {
      Q_OBJECT;
      public:
         struct _SettingConstructParams {
            QString  name;
            QVariant value;
            SettingSerializationType serializationType = SettingSerializationType::Undefined;
         };
         struct _CategoryConstructParams {
            QString name;
            std::initializer_list<_SettingConstructParams> settings;
         };

         File(QObject* parent = nullptr) : QObject(parent) {}
         File(std::initializer_list<_CategoryConstructParams>, QObject* parent = nullptr); // automatically heap-allocate Setting instances

         //
         // e.g.
         // 
         // static File& getMyINIFile() {
         //    static auto* instance = new cobb::qt::ini::File({
         //       { "CategoryName", {
         //          { "bSettingName", false },
         //          { "iSettingName", 5 },
         //          { "xSettingName", MyStruct{}, SettingSerializationType::String },
         //       }},
         //    });
         //    return *instance;
         // }
         //
         
      protected:
         QHash<QString, QVector<Setting*>> _by_category;

         // If a (Setting) is removed because someone called setParent on it for some reason, 
         // take it out of our by-category map. Also adds new settings to the map.
         virtual void childEvent(QChildEvent* event) override;

      public:
         Setting* setting(const QString& category, const QString& name) const noexcept;
         Setting* setting(const QString& name) const noexcept;
         QList<QString> categoryNames() const noexcept;
         QVector<Setting*> settingsByCategory(const QString& category) const noexcept;

         void commitPendingChanges();
         void discardPendingChanges();

         void load(const QString& text);
         QString save(); // also commits any pending changes
         QString save(const QString& old); // attempts to preserve the whitespace, comments, etc., of (old) while writing the new values in place
   };
}