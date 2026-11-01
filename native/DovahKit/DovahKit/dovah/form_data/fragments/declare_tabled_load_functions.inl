namespace dovah::form_data_function_tables::build_outbound_use_info {
   template<>
   struct entry<form_type::DOVAH_FORM_TYPE> {
      static void execute(tes::record_view&, load_order_processes::use_info_build::use_info_builder&);
   };
}
namespace dovah::form_data_function_tables::serialize_form_data {
   extern void load(form_data::DOVAH_FORM_CLASSNAME<managed_form_params>&, tes::record_view&, load_order_interfaces::form_load&);
}