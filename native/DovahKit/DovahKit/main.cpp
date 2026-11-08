#include <QApplication>
#include <QTimer>
#include "lu/cpuinfo.h"

#include "./tests/basic/file_list_to_load_order.h"
#include "./tests/basic/count_all_records.h"
#include "./tests/basic/index_stubs.h"
#include "./tests/basic/load_a_shout.h"
#include "./tests/basic/clear_outbound_uses.h"
#include "./tests/basic/active_file_to_localized.h"

int main(int argc, char *argv[]) {
   lu::cpuinfo::get_or_create();

   QApplication app(argc, argv);
   //QTimer::singleShot(0, &dovahkit::tests::file_list_to_load_order);
   //QTimer::singleShot(0, &dovahkit::tests::count_all_records);
   //QTimer::singleShot(0, &dovahkit::tests::index_stubs);
   //QTimer::singleShot(0, &dovahkit::tests::load_a_shout);
   //QTimer::singleShot(0, &dovahkit::tests::clear_outbound_uses);
   QTimer::singleShot(0, &dovahkit::tests::active_file_to_localized);
   return app.exec();
}
