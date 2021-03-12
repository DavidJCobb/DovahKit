#pragma once
#include <cstdint>
#include <QHash>
#include <QObject>
#include <QString>
#include "../dovah/form_stub.h"

class DovahKitFormDataCache : public QObject {
   Q_OBJECT;
   public:
      static DovahKitFormDataCache& get() noexcept {
         static DovahKitFormDataCache instance;
         return instance;
      }
   protected:
      DovahKitFormDataCache();

      struct {
         QHash<dovah::bare_form_id_t, QString> quest_filters;
         QHash<dovah::bare_form_id_t, QString> static_models;
      } _data;

      static void _parseQuest(const dovah::form_stub&, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc);
      static void _parseStatic(const dovah::form_stub&, dovah::tes_file_reading::record& record, dovah::load_order_interfaces::form_load& intfc);

   protected slots:
      void buildAllData();
      void clear();
      void handleFormChange(dovah::form_stub*);
      void handleFormDelete(dovah::form_stub*, bool will_be_flagged);

   signals:
      void cachedDataBuilt(); // emitted when all data is built
      void cachedDataChanged(dovah::form_stub*, uint32_t code, const QVariant& prior, const QVariant& after); // if !prior.isValid(), then the data was just added
      void cachedDataCleared(); // emitted when all data is cleared
      void cachedDataRemoved(dovah::form_stub*, uint32_t code, const QVariant& data); // emitted only when one form's data is cleared
};