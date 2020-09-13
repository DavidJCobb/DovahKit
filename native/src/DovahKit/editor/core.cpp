#include "core.h"

DovahKitCore::~DovahKitCore() {
   delete this->load_order;
   this->load_order = nullptr;
}
void DovahKitCore::abandon_data() {
   emit dataAbandonImminent();
   delete this->load_order;
   this->load_order = new dovah::file_load_order;
   emit dataAbandonComplete();
}
void DovahKitCore::set_load_order_folder(const std::filesystem::path& p) {
   this->load_order->queued_load.base_path = p.string();
}
void DovahKitCore::queue_load_order_file(const std::filesystem::path& p) {
   this->load_order->queue_file(p.string());
}
void DovahKitCore::unqueue_load_order_file(const std::filesystem::path& p) {
   this->load_order->unqueue_file(p.string());
}
bool DovahKitCore::acquire_load_order_data() {
   auto result = this->load_order->load_queued_files();
   if (result)
      emit dataAcquireComplete();
   else
      emit dataAcquireFailed(this->load_order->load_error);
   return result;
}

const dovah::file_read_error& DovahKitCore::get_last_read_error() const noexcept {
   return this->load_order->load_error;
}

dovah::form_stub* DovahKitCore::get_form(bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(formID);
}
dovah::form_stub* DovahKitCore::get_form(form_type_t ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form(ft, formID);
}
dovah::form_stub* DovahKitCore::get_form_of_probable_type(form_type_t ft, bare_form_id_t formID) const noexcept {
   return this->load_order->get_form_of_probable_type(ft, formID);
}
void DovahKitCore::for_each_form_of_type(form_type_t ft, std::function<bool(dovah::form_stub*)> functor) {
   this->load_order->for_each_form_of_type(ft, functor);
}