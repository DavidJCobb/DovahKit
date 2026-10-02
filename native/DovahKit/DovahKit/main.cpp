#include <QApplication>
#include <QTimer>
#include "lu/cpuinfo.h"

#include "./tests/basic/file_list_to_load_order.h"

int main(int argc, char *argv[]) {
   lu::cpuinfo::get_or_create();

   QApplication app(argc, argv);
   QTimer::singleShot(0, &dovahkit::tests::file_list_to_load_order);
   return app.exec();
}
