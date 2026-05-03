#include "form_data.h"
#include "shout.h"
#include "op_assign.h"
#include "op_clear.h"

namespace dovah {
   class form_stub {
      public:
         int id = 0;
   };
}

#include <iostream>

void print_managed_shout(const dovah::managed_data<dovah::form_data::shout>& data) {
   std::cout << "managed name: " << data.name << '\n';
   for(size_t i = 0; i < data.words.size(); ++i) {
      const auto& word = data.words[i];
      std::cout << "managed spell " << i << ": ";
      if (word.spell) {
         std::cout << word.spell->id;
      } else {
         std::cout << "<none>";
      }
      std::cout << '\n';
   }
}

void test_non_lambda_function(auto& v) {
}

int main() {
   dovah::managed_data<dovah::form_data::shout>   managed_shout;
   dovah::unmanaged_data<dovah::form_data::shout> unmanaged_shout;

   dovah::form_stub spell_a{1};
   dovah::form_stub spell_b{2};
   dovah::form_stub spell_c{3};

   unmanaged_shout.name = "Test Shout";
   unmanaged_shout.words[0].spell = &spell_a;
   unmanaged_shout.words[1].spell = &spell_b;
   unmanaged_shout.words[2].spell = &spell_c;

   std::cout << "Assigning unmanaged data to managed data...\n";

   dovah::form_data_ops::assign(managed_shout, unmanaged_shout);

   std::cout << "unmanaged name: " << unmanaged_shout.name << '\n';
   print_managed_shout(managed_shout);

   std::cout << "Clearing managed form's managed data...\n";

   dovah::form_data_ops::clear_managed_data(managed_shout);

   print_managed_shout(managed_shout);

   std::cout << "Testing non-capturing lambdas...\n";

   // verify that non-capturing lambdas also work for visiting fields
   decltype(unmanaged_shout)::visit_fields(unmanaged_shout, [](auto& field) {
      std::cout << " - visit_fields: visiting a field\n";
   });
   decltype(unmanaged_shout)::visit_fields_in_tandem(unmanaged_shout, managed_shout, [](auto& field, auto& field_b) {
      std::cout << " - visit_fields_in_tandem: visiting a field\n";
   });

   return 0;
}