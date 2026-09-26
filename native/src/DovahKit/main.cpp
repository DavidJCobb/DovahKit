#include <QtWidgets/QApplication>

#include "ui/main_window.h"
#include "editor/core.h"
#include "editor/subsystems/crash_dumper/core.h"

int main(int argc, char* argv[]) {
   dovahkit::subsystems::crash_dumper::core::get_or_create();

   QApplication a(argc, argv);
   a.setWindowIcon(QIcon(":/DovahKit.ico"));
   
   MainWindow w;
   w.show();
   
   auto result = a.exec();
   //
   // Some subsystems have behavioral dependencies on one another, such that we don't want to 
   // count on construction/destruction order to resolve them. It seems safest to exit with a 
   // data-abandon step, same as if the user unloaded data (e.g. to switch to editing another 
   // file) while running the program.
   // 
   // [2/27/2024] Without this call, closing the program after loading data causes assertion 
   // failures within the Papyrus subsystem: the form-info-cache subsystem doesn't clear out 
   // refcounted pointers to known Papyrus scripts (it only does that on-data-abandon), and 
   // then when the Papyrus subsystem is destroyed and it deletes its known scripts, it will 
   // fail assertions as to their refcounts being zero. We could give the form-info-cache a 
   // destructor that severs those pointers, but guaranteeing that the subsystems are both 
   // destroyed in the right order is... It feels very "action at a distance"-y. It mirrors 
   // construction order, so we could have the Papyrus subsystem be what initially constructs 
   // the form-info-cache subsystem, or change the order in which DovahKitCore constructs 
   // both of them; but it feels easier to just forcibly abandon game data and rely on signals 
   // and slots so we don't have to worry about it.
   // 
   // If this results in exit being too slow, we could look into writing quick-exit handlers 
   // for subsystems as needed and then doing std::quick_exit here.
   //
   DovahKitCore::get().abandon_data();
   //
   return result;
}