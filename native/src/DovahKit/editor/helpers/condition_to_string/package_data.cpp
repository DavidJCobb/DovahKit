#include "./package_data.h"
#include <QCoreApplication>
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/structs/typed_package_info/custom.h"
#include "dovah/forms/Package.h"

namespace {
   using modern_package_info = dovah::loaded_forms::structs::typed_package_info::custom;

   static QString _packdata_name(const dovah::loaded_forms::structs::custom_packages::package_data_declaration_map& decls, uint32_t packdata_uid) {
      for (auto& entry : decls.entries)
         if (entry.unique_id == packdata_uid)
            return QString::fromStdString(entry.name);
      return {};
   }
}

namespace editor_helpers::condition_to_string {
   extern std::pair<QString, bool> package_data(
      const dovah::loaded_forms::components::conditions::context& context,
      uint32_t packdata_uid
   ) {
      if (packdata_uid == 0xFF || packdata_uid == -1)
         return { QCoreApplication::translate("condition use of packdata", "NONE", "no packdata"), false };
      const auto* package = context.get_owning_package();
      if (package) {
         QString name;
         const auto* custom = dynamic_cast<modern_package_info*>(package->typed_info);
         if (custom) {
            if (auto* tp_stub = custom->template_package.get_form_stub(); tp_stub) {
               auto loaded = tp_stub->load().ptr_cast<dovah::loaded_forms::Package>();
               if (loaded) {
                  if (const auto* inherited = dynamic_cast<modern_package_info*>(loaded->typed_info))
                     name = _packdata_name(inherited->data.declarations, packdata_uid);
               }
            } else {
               name = _packdata_name(custom->data.declarations, packdata_uid);
            }
         }
         if (!name.isEmpty())
            return { name, true };
      }
      return { QCoreApplication::translate("condition use of packdata", "Package Data #%1", "package not found").arg(packdata_uid), false };
   }
}
