#include "get_record_size_stats.h"
#include <QMessageBox>
#include "../../../dovah/form_stub.h"
#include "../../../editor/core.h"

namespace DovahKitDebug::features {
   /*static*/ void get_record_size_stats::execute(QWidget* window) {
      struct deets {
         uint32_t smallest = std::numeric_limits<uint32_t>::max();
         uint32_t largest  = 0;
         uint32_t total    = 0;
         uint32_t count    = 0;
         inline float average() const noexcept { return (float)total / (float)count; }
         void consume(uint32_t size) noexcept {
            total += size;
            ++count;
            if (size < smallest)
               smallest = size;
            if (size > largest)
               largest = size;
         }
         QString report() const noexcept {
            return QString("Range: [%1, %2]; average %3 over %4 records.").arg(smallest).arg(largest).arg(average()).arg(count);
         }
      };
      //
      deets compressed;
      deets uncompressed;
      auto& editor = DovahKitCore::get();
      editor.for_each_form([&compressed, &uncompressed](const dovah::form_stub* stub) {
         dovah::tes_file_record_header header;
         uint32_t decompressed_size;
         if (stub->fetch_record_header(header, decompressed_size)) {
            if (header.body_is_compressed()) {
               compressed.consume(decompressed_size);
            } else {
               uncompressed.consume(header.size);
            }
         }
         return false;
      });
      QMessageBox::question(window,
         QObject::tr("Report", "debug"),
         QObject::tr("Compressed records:<br/>%1<br/><br/>Uncompressed records:<br/>%2", "debug").arg(compressed.report()).arg(uncompressed.report()),
         QMessageBox::Ok
      );
   }
}