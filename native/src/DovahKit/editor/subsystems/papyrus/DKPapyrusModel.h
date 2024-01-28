#pragma once
#include <array>
#include <optional>
#include <QAbstractItemModel>
#include <QString>
#include "dovah/form_types.h"

namespace dovah {
   class compiled_papyrus_script;
}

class DKPapyrusModel : public QAbstractItemModel {
   Q_OBJECT;
   protected:
      class Script {
         public:
            struct Info {
               QString docstring;
               struct {
                  QString name;
                  Script* target   = nullptr; // NOTE: We do not create `Script` instances for hardcoded or SKSE types.

                  // REMINDER: The value `dovah::form_type::none` represents the base Form class.
                  // REMINDER: This enumeration also has values for aliases.
                  std::optional<dovah::form_type_t> underlying_type;
               } extends;
               struct {
                  bool conditional = false;
                  bool hidden      = false;
               } flags;
            };

         public:
            QString name;
            struct {
               std::optional<Info> packed;
               std::optional<Info> loose;
            } info;
            struct {
               bool cyclical = false;

               Script* root_class = nullptr;
               struct {
                  QVector<Script*> loose;
                  QVector<Script*> packed;
               } potential_subclasses;
            } inheritance;

            constexpr bool exists() const noexcept {
               return info.packed.has_value() || info.loose.has_value();
            }

            int compare_name(QString) const;
            bool name_matches(QString) const;
            const Script* superclass() const;
            Script* superclass();

            bool is_of_type(dovah::form_type_t) const;
            bool is_of_type(QString desired) const; // DOES NOT handle hardcoded scriptnames representing form/alias types.
            std::optional<dovah::form_type_t> underlying_type() const;

            bool is_unreferenced() const;

            void receive_subclass(Script& subclass, bool loose);
            void abandon_loose_subclass(Script& subclass);

            // Non-recursive; used when initially loading all scripts.
            void _compute_root_class();

            // Recursive: used when the class hierarchy has changed (e.g. because a loose file was added/edited/deleted).
            void _update_descendants_root_class();
      };

      class script_collection {
         public:
            static constexpr const size_t index_of_none = (size_t)-1;

         protected:
            QVector<Script*>       scripts;
            std::array<size_t, 27> counts = {}; // number of scripts whose names start with the given alphabetical character. offset #0 is "before alphabetical"

         protected:
            static size_t _which_count(QString name);

         public:
            const Script* at(size_t) const;
            Script* at(size_t);

            void clear(); // deletes contents

            size_t index_of(QString name) const;
            const Script* lookup(QString name) const;
            Script* lookup(QString name);
            void insert(Script*); // asserts that the passed-in script isn't already stored

            constexpr const QVector<Script*>& list() const noexcept { return this->scripts; }
            constexpr size_t size() const noexcept { return list().size(); }

            void take(Script&); // asserts that the passed-in script is already stored
      };

   public:
      DKPapyrusModel(QObject* parent);
      ~DKPapyrusModel();

   protected:
      script_collection _data;

      Script* _scan_pex(script_collection& dst, const std::string& filename, const void* src_data, const size_t src_size, bool is_loose);

      void _update_superclass_of(Script&);

      void _on_loose_file_created(QString scriptname);
      void _on_loose_file_edited(QString scriptname);
      void _on_loose_file_deleted(QString scriptname);

   public:
      static std::optional<dovah::form_type_t> type_of_hardcoded_scriptname(QString);

      void populate_initial();

      bool script_is_of_type(QString subject, dovah::form_type_t desired) const;
      bool script_is_of_type(QString subject, QString desired) const;

      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual int         rowCount(const QModelIndex& parent) const override final;
            virtual int         columnCount(const QModelIndex& item) const override final;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
      #pragma endregion
};