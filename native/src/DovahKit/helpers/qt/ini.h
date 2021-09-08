/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <concepts>
#include <QDir>
#include <QObject>
#include <QPointer>
#include <QVariant>
#include "../passkey.h"

// 
// USAGE GUIDE:
// 
//  = The "ini" subfolder contains helper files as well as example code.
// 
//  - Settings have an "initial" value, which is an executable-level default; a "current" value, which is the 
//    actual value of the setting at the present moment; and a "pending" value, which exists as an aid to code 
//    which wishes to make changes to settings with the option to commit or discard those changes later (e.g. 
//    a program options UI). Each of these is stored, internally, as a QVariant.
// 
//    Writing an invalid QVariant into the current value resets it to the default (i.e. the current value is 
//    overwritten with a copy of the initial value). Writing an invalid QVariant into the pending value discards 
//    the pending value, if there was a value (i.e. if the pending value wasn't already an invalid QVariant).
// 
//    A file's pending values can be committed en masse, or they can be committed individually. You can also 
//    directly set a setting's current value, though this is better for things other than options UI.
// 
//  - Settings can store structs as their values; however, these structs must be made compatible with QVariant, 
//    and Qt must know how to convert the struct types to and from QString. This means that you must...
// 
//     - Use the Q_DECLARE_METATYPE macro on the type.
// 
//     - Ensure that QMetaType::registerConverter is called to register conversion functions, before the INI 
//       file is ever loaded or its settings are ever written to. You must make two calls in order to register 
//       converters in both directions.
// 
//        - If your type has a member function which converts to a QString, then you can pass a pointer to that 
//          member function as an argument to one of the QMetaType::registerConverter calls, with no further 
//          work required.
// 
//        - You will need to define a non-member function which takes a const QString& argument and returns an 
//          instance of your struct, and then call QMetaType::registerConverter<QString, YourType>(&YourFunction).
// 
//    Doing things this way, however, allows you to mostly automate conversion between your struct and a string, 
//    and it allows you to store an instance of your struct directly in the setting's QVariant (instead of having 
//    to cache the struct instance elsewhere e.g. as a member on a program options dialog, or alternatively, 
//    convert back and forth between the string representation with each change to the value).
// 
// NOTES:
// 
//  - Code which performs some long task in response to a setting being changed (e.g. changing the application 
//    stylesheet) is encouraged to use a Qt::QueuedConnection to listen for the setting change. This way, if the 
//    application commits pending changes and then saves the INI file, it can be sure that the file will be saved 
//    before the long-running task happens.
// 
//  - Static instances of a QObject (outside of a function scope) are vulnerable to the static initialization 
//    order fiasco: the QObject instances can be created before their class's QMetaObject (Qt RTTI) has been 
//    created, and this breaks virtually all QObject functionality.
// 
//    What's more, the QObject hierarchy system isn't strictly compatible with non-heap-allocated instances of a 
//    QObject: if a static QObject instance has a parent set on it through any means, then the parent will try to 
//    delete it when the parent is destroyed. Deleting something that was never actually heap-allocated will cause 
//    a crash... if you're lucky.
// 
//    These considerations affect both Setting objects and File objects.
// 
//     - The File() constructor that takes setting definitions can help. We can have a singleton-style getter 
//       function for an INI file which heap-allocates a File with setting definitions; that'll also create 
//       its settings; and then we can...
// 
//        - ...only ever access settings through the File.
// 
//        - ...have specific pieces of code that need a setting use their own getter functions, which get the 
//          setting through the file and then cache the setting pointer in a static local variable for faster 
//          access later.
// 
//     - We also want to support constructing INI file specs at run-time. I want to be able to define all of 
//       Skyrim's INI settings and their default values within the `dovah` folder in a Qt-independent manner, 
//       even if I have the frontend, Dovahscript, render window, etc., all rely on this system here to actually 
//       load and work with Skyrim.ini.
// 
//  - Arbitrary structs are supported as values given the following details:
// 
//     - The values are serialized as strings.
// 
//     - You must register the struct type using Q_DECLARE_METATYPE.
// 
//     - You must register a "converter" from the struct to QString using either of these methods:
// 
//        - QMetaType::registerConverter(&T::toString);
//        - QMetaType::registerConverter(&myStructToString); // given: extern QString myStructToString(const T&);
// 
//     - You must register a "converter" from QString to the struct using:
// 
//        - QMetaType::registerConverter(&stringToMyStruct); // given: extern T stringToMyString(const QString&);
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
         const int qMetaTypeID;
      protected:
         struct {
            const QVariant initial;
            QVariant current;
            QVariant pending; // invalid variant == no pending changes
         } values;

         bool _typeCheckValue(const QVariant&) const noexcept;

      public:
         File* file() const noexcept;

         inline QVariant initialValue() const noexcept { return this->values.initial; }
         inline QVariant currentValue() const noexcept { return this->values.current; }
         inline QVariant pendingValue() const noexcept { return this->values.pending; }
         void setCurrentValue(const QVariant&) noexcept; // an invalid variant resets the value to its initial. a variant not convertible to the setting's qMetaTypeID is ignored and no change is made.
         void setPendingValue(const QVariant&) noexcept; // an invalid variant clears the pending value. a variant not convertible to the setting's qMetaTypeID is ignored and no change is made.

         inline bool hasPendingValue() const noexcept { return this->pendingValue().isValid(); }
         inline QVariant pendingOrCurrentValue() const noexcept {
            auto data = this->pendingValue();
            if (!data.isValid())
               data = this->currentValue();
            return data;
         }

         template<typename T> requires (!std::constructible_from<QVariant, T>) void setCurrentValue(T value) {
            this->setCurrentValue(QVariant::fromValue<T>(value));
         }
         template<typename T> requires (!std::constructible_from<QVariant, T>) void setPendingValue(T value) {
            this->setPendingValue(QVariant::fromValue<T>(value));
         }

         // Helper for struct-type settings. Call setting->modifyPendingValue<StructType>(...) passing 
         // a function pointer or lambda as an argument; the function should take a non-const reference 
         // to a struct to modify, and return void.
         template<typename T, typename functor_type> requires std::is_invocable_v<functor_type, T&>
         void modifyPendingValue(functor_type f) {
            auto data  = this->pendingOrCurrentValue();
            auto value = data.isValid() ? data.value<T>() : T{};
            f(value);
            this->setPendingValue(value);
         }

         void discardPendingValue();
         void commitPendingValue();

         void load(QStringRef);
         QString currentValueString() const noexcept;

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
         struct _FileConstructParams {
            QString path;
            std::initializer_list<_CategoryConstructParams> categories;
         };

         File(QObject* parent = nullptr) : QObject(parent) {}
         File(_FileConstructParams, QObject* parent = nullptr); // automatically heap-allocate Setting instances

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
         QHash<QString, QVector<QPointer<Setting>>> _by_category;
         QVector<QPointer<Setting>> _all_settings;
         QChar _comment_char = ';';
         QString _path;

      public:
         void _addSetting(cobb::passkey<File, Setting>, Setting&);

         Setting* setting(const QString& category, const QString& name, Qt::CaseSensitivity) const noexcept;
         Setting* setting(const QString& category, const QString& name) const noexcept;
         Setting* setting(const QString& name) const noexcept;
         QList<QString> categoryNames() const noexcept;
         QVector<Setting*> settingsByCategory(const QString& category) const noexcept;

         QString categoryNameCanonicalCase(const QString& name) const noexcept; // given a category named "FooBar", converts "FoObAr", "foobar", etc., to "FooBar"; returns empty string if category doesn't exist

         void commitPendingChanges();
         void discardPendingChanges();

         inline QString path() const noexcept { return this->_path; }
         void setPath(QString);
         
         // Load/save functions require that there be a non-empty path. They're provided for convenience; 
         // you can just as easily call the import/export functions yourself and manage QFiles, etc., to 
         // get the data where it needs to go.
         bool load();
         bool save(bool preserve_formatting = false);

         void importFromString(const QString& text);
         QString exportToString();
         QString exportToString(const QString& old); // given existing INI file content (old), attempts to preserve the whitespace, comments, order, etc., of (old) while writing the new values in place and adding any missing data
   };

   // This concept matches any function which takes no arguments and returns a File&.
   template<typename T> concept IsFileGetter = requires(T g) { { g() } -> std::same_as<File&>; };

   //
   // Helper class for references to a setting, when your code plans on using the setting 
   // frequently. Given a FileGetter (which would presumably lazy-construct the File in a 
   // manner similar to a Meyers singleton) and the setting category and name, this class 
   // will retrieve the setting on demand and then cache it for later. Essentially, you 
   // can do:
   // 
   //    // given static File& GetMainINI():
   //    static SettingPointer sMySetting = SettingPointer(&GetMainINI, "General", "sMySetting");
   // 
   // and then you'll be able to retrieve the setting the first time you access sMySetting, 
   // with the class caching the pointer for you automatically.
   // 
   // You can alternatively also do the following, but doing it across your code may cause 
   // a lot of lookups to happen all at once, on startup:
   // 
   //    static Setting* sMySetting = GetMainINI().setting("General", "sMySetting");
   //
   template<typename FileGetter> requires IsFileGetter<FileGetter>
   class SettingPointer {
      protected:
         QPointer<Setting> setting = nullptr;
         //
         const FileGetter getter;
         const QString    name;
         const QString    category;

      public:
         SettingPointer(FileGetter g, const QString& category, const QString& name) : getter(g), name(name), category(category) {}

         Setting* get() {
            if (this->setting)
               return this->setting;
            this->setting = (this->getter()).setting(this->category, this->name);
         }

         Setting* operator->() { return this->get(); }
         Setting& operator*() { return *this->get(); }
   };
}