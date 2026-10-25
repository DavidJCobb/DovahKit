#include <QApplication>
#include <QTimer>
#include "lu/cpuinfo.h"

#include "./tests/basic/file_list_to_load_order.h"
#include "./tests/basic/count_all_records.h"
#include "./tests/basic/index_stubs.h"

int main(int argc, char *argv[]) {
   lu::cpuinfo::get_or_create();

   QApplication app(argc, argv);
   //QTimer::singleShot(0, &dovahkit::tests::file_list_to_load_order);
   //QTimer::singleShot(0, &dovahkit::tests::count_all_records);
   QTimer::singleShot(0, &dovahkit::tests::index_stubs);
   return app.exec();
}
